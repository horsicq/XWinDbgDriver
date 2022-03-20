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
#ifndef XWINDBGDRIVER_H
#define XWINDBGDRIVER_H

#include <QObject>
#include <Windows.h>

class XWinDbgDriver : public QObject
{
    Q_OBJECT

public:
    explicit XWinDbgDriver(QObject *pParent=nullptr);

    bool loadDriver(QString sFileName,QString sServiceName="X_KERNEL_DRIVER");
    qint32 readMemory();

private:
    bool installDriver(SC_HANDLE hSCManager,QString sServiceName,QString sFileName);
    bool removeDriver(SC_HANDLE hSCManager,QString sServiceName);
    bool startDriver(SC_HANDLE hSCManager,QString sServiceName);

signals:
    void infoMessage(QString sText);
    void errorMessage(QString sText);
};

#endif // XWINDBGDRIVER_H
