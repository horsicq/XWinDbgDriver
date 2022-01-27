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

bool XWinDbgDriver::loadDriver(QString sFileName, QString sServiceName)
{
    bool bResult=false;

    SC_HANDLE hSCManager=0;
    SC_HANDLE hService=0;
    HANDLE hDevice=0;

    hSCManager=OpenSCManagerW(0,0,SC_MANAGER_ALL_ACCESS);

    // TODO errors
    if(hSCManager)
    {
        // remove driver
        // install driver
        // start driver
        // TODO Create Service
    }
    else
    {
    #ifdef QT_DEBUG
        qDebug("Cannot open SCManager");
    #endif
    }

    return bResult;
}

