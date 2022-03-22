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

}

HANDLE XWinDbgDriver::loadDriver(QString sFileName, QString sServiceName)
{
    HANDLE nResult=0;

    SC_HANDLE hSCManager=OpenSCManagerW(NULL,NULL,SC_MANAGER_ALL_ACCESS);

    // TODO errors
    if(hSCManager)
    {
        removeDriver(hSCManager,sServiceName);
        installDriver(hSCManager,sServiceName,sFileName);

        if(startDriver(hSCManager,sServiceName))
        {
            nResult=openDevice(sServiceName);
        }

        CloseServiceHandle(hSCManager);
    }
    else
    {
    #ifdef QT_DEBUG
        qDebug("%s",XProcess::getLastErrorAsString().toUtf8().data());
    #endif
    }

    return nResult;
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
    }

    return bResult;
}

bool XWinDbgDriver::removeDriver(SC_HANDLE hSCManager, QString sServiceName)
{
    bool bResult=false;

    SC_HANDLE hSCService=OpenServiceW(hSCManager,(LPCWSTR)(sServiceName.utf16()),SERVICE_ALL_ACCESS);

    if(hSCService)
    {
        bResult=DeleteService(hSCService);

        CloseServiceHandle(hSCService);
    }
    else
    {
    #ifdef QT_DEBUG
        qDebug("%s",XProcess::getLastErrorAsString().toUtf8().data());
    #endif
    }

    return bResult;
}

bool XWinDbgDriver::startDriver(SC_HANDLE hSCManager, QString sServiceName)
{
    bool bResult=false;

    SC_HANDLE hSCService=OpenService(hSCManager,(LPCWSTR)(sServiceName.utf16()),SERVICE_ALL_ACCESS);

    if(hSCService)
    {
        BOOL _bResult=StartServiceW(hSCService,0,NULL);

        bResult=(_bResult==TRUE);

        CloseServiceHandle(hSCService);
    }
    else
    {
    #ifdef QT_DEBUG
        qDebug("%s",XProcess::getLastErrorAsString().toUtf8().data());
    #endif
    }

    return bResult;
}

bool XWinDbgDriver::stopDriver(QString sServiceName)
{
    bool bResult=false;


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
    }

    return hResult;
}

