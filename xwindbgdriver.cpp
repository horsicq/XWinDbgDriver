/* Copyright (c) 2021-2022 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "xwindbgdriver.h"

XWinDbgDriver::XWinDbgDriver(QObject *pParent) : QObject(pParent)
{
    g_hDriver=0;
}

XWinDbgDriver::~XWinDbgDriver()
{
    unload();
}

bool XWinDbgDriver::load(QString sDriverFolder)
{
    unload();

    bool bResult=false;

    if(sDriverFolder!="")
    {
    #if defined(Q_PROCESSOR_X86_32)
        QString sDriverFileName=sDriverFolder+QDir::separator()+"x86\\1394\\1394kdbg.sys";
    #elif defined(Q_PROCESSOR_X86_64)
        QString sDriverFileName=sDriverFolder+QDir::separator()+"x64\\1394\\1394kdbg.sys";
    #elif defined(Q_PROCESSOR_ARM_32)
        QString sDriverFileName=sDriverFolder+QDir::separator()+"arm\\1394\\1394kdbg.sys";
    #elif defined(Q_PROCESSOR_ARM_64)
        QString sDriverFileName=sDriverFolder+QDir::separator()+"arm64\\1394\\1394kdbg.sys";
    #endif

        if(XBinary::isFileExists(sDriverFileName))
        {
//            g_sServiceName=QString("X_KERNEL_DRIVER_%1").arg(XBinary::random32());
            g_sServiceName="X_KERNEL_DRIVER_A";

            g_hDriver=loadDriver(sDriverFileName,g_sServiceName);

            if(g_hDriver)
            {
                bResult=true;
            }
        }
        else
        {
            emit errorMessage(QString("%1: %2").arg(tr("Cannot find file"),sDriverFileName));
        }
    }

    return bResult;
}

void XWinDbgDriver::unload()
{
    if(g_hDriver!=0)
    {
        CloseHandle(g_hDriver);

        g_hDriver=0;
    }

    if(g_sServiceName!="")
    {
        unloadDriver(g_sServiceName);

        g_sServiceName="";
    }
}

quint64 XWinDbgDriver::readMemory(quint64 nAddress, char *pData, quint64 nDataSize)
{
    quint64 nResult=0;

    if(g_hDriver)
    {
        S_KLDBG kldbg={};
        IO_STATUS_BLOCK iost={};
        S_SYSDBG_VIRTUAL dbgRequest={};

        dbgRequest.Address=(PVOID)nAddress;
        dbgRequest.Buffer=pData;
        dbgRequest.Request=nDataSize;

        kldbg.SysDbgRequest=SysDbgReadVirtual;
        kldbg.Buffer=&dbgRequest;
        kldbg.BufferSize=sizeof(S_SYSDBG_VIRTUAL);

        iost.Information=0;
        iost.Status=0;

        NTSTATUS ntStatus=NtDeviceIoControlFile(g_hDriver,
                                                NULL,
                                                NULL,
                                                NULL,
                                                &iost,
                                                CTL_CODE(FILE_DEVICE_UNKNOWN,0x1,METHOD_NEITHER,FILE_READ_ACCESS|FILE_WRITE_ACCESS),
                                                &kldbg,
                                                sizeof(kldbg),
                                                &dbgRequest,
                                                sizeof(dbgRequest));

        if(ntStatus==STATUS_PENDING)
        {
            ntStatus=NtWaitForSingleObject(g_hDriver,FALSE,NULL);
        }

        if(NT_SUCCESS(ntStatus))
        {
            nResult=iost.Information;
        }
        else
        {
            // TODO error
        }
    }

    return nResult;
}

HANDLE XWinDbgDriver::loadDriver(QString sFileName, QString sServiceName)
{
    HANDLE hResult=0;

    SC_HANDLE hSCManager=OpenSCManagerW(NULL,NULL,SC_MANAGER_ALL_ACCESS);

    if(hSCManager)
    {
        removeDriver(hSCManager,sServiceName);
        installDriver(hSCManager,sServiceName,sFileName);

        if(startDriver(hSCManager,sServiceName))
        {
            hResult=openDevice(sServiceName);
        }

        CloseServiceHandle(hSCManager);
    }
    else
    {
    #ifdef QT_DEBUG
        qDebug("%s",XProcess::getLastErrorAsString().toUtf8().data());
    #endif
        emit errorMessage(QString("openDevice::loadDriver: %1").arg(XProcess::getLastErrorAsString()));
    }

    return hResult;
}

bool XWinDbgDriver::unloadDriver(QString sServiceName)
{
    bool bResult=false;

    SC_HANDLE hSCManager=OpenSCManagerW(NULL,NULL,SC_MANAGER_ALL_ACCESS);

    if(hSCManager)
    {
        stopDriver(hSCManager,sServiceName);
        bResult=removeDriver(hSCManager,sServiceName);
        CloseServiceHandle(hSCManager);
    }
    else
    {
    #ifdef QT_DEBUG
        qDebug("%s",XProcess::getLastErrorAsString().toUtf8().data());
    #endif
        emit errorMessage(QString("XWinDbgDriver::unloadDriver: %1").arg(XProcess::getLastErrorAsString()));
    }

    return bResult;
}

bool XWinDbgDriver::installDriver(SC_HANDLE hSCManager, QString sServiceName, QString sFileName)
{
    bool bResult=false;

    SC_HANDLE hSCService=CreateServiceW(hSCManager,                         // SCManager database
                                        (LPCWSTR)(sServiceName.utf16()),    // name of service
                                        (LPCWSTR)(sServiceName.utf16()),    // name to display
                                        SERVICE_ALL_ACCESS,                 // desired access
                                        SERVICE_KERNEL_DRIVER,              // service type
                                        SERVICE_DEMAND_START,               // start type
                                        SERVICE_ERROR_NORMAL,               // error control type
                                        (LPCWSTR)(sFileName.utf16()),       // service's binary
                                        NULL,                               // no load ordering group
                                        NULL,                               // no tag identifier
                                        NULL,                               // no dependencies
                                        NULL,                               // LocalSystem account
                                        NULL);                              // no password

    if(hSCService)
    {
        bResult=true;

        CloseServiceHandle(hSCService);
    }
    else
    {
    #ifdef QT_DEBUG
        qDebug("%s",XProcess::getLastErrorAsString().toUtf8().data());
    #endif
        emit errorMessage(QString("XWinDbgDriver::installDriver: %1").arg(XProcess::getLastErrorAsString()));
    }

    return bResult;
}

bool XWinDbgDriver::removeDriver(SC_HANDLE hSCManager, QString sServiceName)
{
    bool bResult=false;

    SC_HANDLE hSCService=OpenServiceW(hSCManager,(LPCWSTR)(sServiceName.utf16()),SERVICE_ALL_ACCESS);

    if(hSCService)
    {
        bResult=(DeleteService(hSCService)==TRUE);

        CloseServiceHandle(hSCService);
    }
    else
    {
    #ifdef QT_DEBUG
        qDebug("%s",XProcess::getLastErrorAsString().toUtf8().data());
    #endif
        emit errorMessage(QString("XWinDbgDriver::removeDriver: %1").arg(XProcess::getLastErrorAsString()));
    }

    return bResult;
}

bool XWinDbgDriver::startDriver(SC_HANDLE hSCManager, QString sServiceName)
{
    bool bResult=false;

    SC_HANDLE hSCService=OpenService(hSCManager,(LPCWSTR)(sServiceName.utf16()),SERVICE_ALL_ACCESS);

    if(hSCService)
    {
        bResult=(StartServiceW(hSCService,0,NULL)==TRUE);

        if(!bResult)
        {
        #ifdef QT_DEBUG
            qDebug("%s",XProcess::getLastErrorAsString().toUtf8().data());
        #endif
            emit errorMessage(QString("XWinDbgDriver::startDriver: %1").arg(XProcess::getLastErrorAsString()));
        }

        CloseServiceHandle(hSCService);
    }
    else
    {
    #ifdef QT_DEBUG
        qDebug("%s",XProcess::getLastErrorAsString().toUtf8().data());
    #endif
        emit errorMessage(QString("XWinDbgDriver::startDriver: %1").arg(XProcess::getLastErrorAsString()));
    }

    return bResult;
}

bool XWinDbgDriver::stopDriver(SC_HANDLE hSCManager, QString sServiceName)
{
    bool bResult=false;

    SC_HANDLE hSCService=OpenService(hSCManager,(LPCWSTR)(sServiceName.utf16()),SERVICE_ALL_ACCESS);

    if(hSCService)
    {
        for(qint32 i=0;i<5;i++)
        {
            SERVICE_STATUS serviceStatus={0};

            SetLastError(ERROR_SUCCESS);

            if(ControlService(hSCService,SERVICE_CONTROL_STOP,&serviceStatus)==TRUE)
            {
                bResult=true;

                break;
            }

            if(GetLastError()!=ERROR_DEPENDENT_SERVICES_RUNNING)
            {
                break;
            }

            Sleep(1000);
        }

        CloseServiceHandle(hSCService);
    }

    if(!bResult)
    {
    #ifdef QT_DEBUG
        qDebug("%s",XProcess::getLastErrorAsString().toUtf8().data());
    #endif
        emit errorMessage(QString("XWinDbgDriver::stopDriver: %1").arg(XProcess::getLastErrorAsString()));
    }

    return bResult;
}

HANDLE XWinDbgDriver::openDevice(QString sServiceName)
{
    HANDLE hResult=0;

    QString sCompleteDeviceName=QString("\\\\.\\%1").arg(sServiceName);

    HANDLE hDevice=CreateFileW((LPCWSTR)(sCompleteDeviceName.utf16()),
                GENERIC_READ|GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

    if(hDevice!=INVALID_HANDLE_VALUE)
    {
        hResult=hDevice;
    }
    else
    {
    #ifdef QT_DEBUG
        qDebug("%s",XProcess::getLastErrorAsString().toUtf8().data());
    #endif
        emit errorMessage(QString("openDevice::stopDriver: %1").arg(XProcess::getLastErrorAsString()));
    }

    return hResult;
}

