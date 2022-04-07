/* Copyright (c) 2022 hors<horsicq@gmail.com>
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
#ifndef XINFODB_H
#define XINFODB_H

#include <QObject>
#include "xbinary.h"
#include "xprocess.h" // TODO def

class XInfoDB : public QObject
{
    Q_OBJECT
public:

    enum MODE
    {
        MODE_UNKNOWN=0,
        MODE_PROCESS
    };

    struct REG_OPTIONS
    {
        bool bGeneral;
        bool bIP;
        bool bFlags;
        bool bSegments;
        bool bDebug;
        bool bFloat;
        bool bXMM;
    };

    enum REG
    {
        REG_UNKNOWN=0,
        REG_EAX,
        REG_ECX,
        REG_EDX,
        REG_EBX,
        REG_ESP,
        REG_EBP,
        REG_ESI,
        REG_EDI,
        REG_EIP,
        REG_RAX,
        REG_RCX,
        REG_RDX,
        REG_RBX,
        REG_RSP,
        REG_RBP,
        REG_RSI,
        REG_RDI,
        REG_R8,
        REG_R9,
        REG_R10,
        REG_R11,
        REG_R12,
        REG_R13,
        REG_R14,
        REG_R15,
        REG_RIP,
        REG_EFLAGS,
        REG_CS,
        REG_DS,
        REG_ES,
        REG_FS,
        REG_GS,
        REG_SS,
        REG_DR0,
        REG_DR1,
        REG_DR2,
        REG_DR3,
        REG_DR6,
        REG_DR7,
        REG_CF,
        REG_PF,
        REG_AF,
        REG_ZF,
        REG_SF,
        REG_TF,
        REG_IF,
        REG_DF,
        REG_OF,
        REG_ST0,
        REG_ST1,
        REG_ST2,
        REG_ST3,
        REG_ST4,
        REG_ST5,
        REG_ST6,
        REG_ST7,
        REG_XMM0,
        REG_XMM1,
        REG_XMM2,
        REG_XMM3,
        REG_XMM4,
        REG_XMM5,
        REG_XMM6,
        REG_XMM7,
        REG_XMM8,
        REG_XMM9,
        REG_XMM10,
        REG_XMM11,
        REG_XMM12,
        REG_XMM13,
        REG_XMM14,
        REG_XMM15,
    };

    explicit XInfoDB(QObject *pParent=nullptr);
    ~XInfoDB();

    void setProcess(XProcess::HANDLEID hidProcess);
    void updateRegs(XProcess::HANDLEID hidThread,REG_OPTIONS regOptions);
    void updateMemoryRegionsList();
    void updateModulesList();
    XBinary::XVARIANT getCurrentReg(REG reg);
    QList<XBinary::MEMORY_REGION> *getCurrentMemoryRegionsList();
    QList<XBinary::MODULE> *getCurrentModulesList();
    bool isRegChanged(REG reg);

    static QString regIdToString(REG reg);

private:

    struct STATUS
    {
        QMap<REG,XBinary::XVARIANT> mapRegs;
        QList<XBinary::MEMORY_REGION> listMemoryRegions;
        QList<XBinary::MODULE> listModules;
    };

    XBinary::XVARIANT _getReg(QMap<REG,XBinary::XVARIANT> *pMapRegs,REG reg);

private:
    XProcess::HANDLEID g_hidProcess;
    MODE g_mode;

    STATUS g_statusCurrent;
    STATUS g_statusPrev;
};

#endif // XINFODB_H
