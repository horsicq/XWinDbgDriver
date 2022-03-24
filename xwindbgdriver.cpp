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
    if(g_hDriver)
    {
       CloseHandle(g_hDriver);
    }
}

bool XWinDbgDriver::load()
{
    g_hDriver=openDevice("kldbgdrv");

    return (g_hDriver!=0);
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
        emit errorMessage(QString("XWinDbgDriver::stopDriver: %1").arg(XProcess::getLastErrorAsString()));
    }

    return hResult;
}

