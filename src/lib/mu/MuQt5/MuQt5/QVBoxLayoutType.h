//
// Copyright (c) 2009, Jim Hourihan
// All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef __MuQt5__QVBoxLayoutType__h__
#define __MuQt5__QVBoxLayoutType__h__
#include <iostream>
#include <Mu/Class.h>
#include <Mu/MuProcess.h>
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>
#include <QtNetwork/QtNetwork>
#include <QtWebEngine/QtWebEngine>
#include <QtWebEngineWidgets/QtWebEngineWidgets>
#include <QtQml/QtQml>
#include <QtQuick/QtQuick>
#include <QtQuickWidgets/QtQuickWidgets>
#include <QtSvg/QtSvg>
#include <MuQt5/Bridge.h>

namespace Mu
{
    class MuQt_QVBoxLayout;

    //
    //  NOTE: file generated by qt2mu.py
    //

    class QVBoxLayoutType : public Class
    {
    public:
        typedef MuQt_QVBoxLayout MuQtType;
        typedef QVBoxLayout QtType;

        //
        //  Constructors
        //

        QVBoxLayoutType(Context* context, const char* name,
                        Class* superClass = 0, Class* superClass2 = 0);

        virtual ~QVBoxLayoutType();

        static bool isInheritable() { return true; }

        static inline ClassInstance* cachedInstance(const MuQtType*);

        //
        //  Class API
        //

        virtual void load();

        MemberFunction* _func[13];
    };

    // Inheritable object

    class MuQt_QVBoxLayout : public QVBoxLayout
    {
    public:
        virtual ~MuQt_QVBoxLayout();
        MuQt_QVBoxLayout(Pointer muobj, const CallEnvironment*);
        MuQt_QVBoxLayout(Pointer muobj, const CallEnvironment*,
                         QWidget* parent);
        virtual void addItem(QLayoutItem* item);
        virtual int count() const;
        virtual Qt::Orientations expandingDirections() const;
        virtual bool hasHeightForWidth() const;
        virtual int heightForWidth(int w) const;
        virtual void invalidate();
        virtual QLayoutItem* itemAt(int index) const;
        virtual QSize maximumSize() const;
        virtual int minimumHeightForWidth(int w) const;
        virtual QSize minimumSize() const;
        virtual void setGeometry(const QRect& r);
        virtual QSize sizeHint() const;
        virtual QLayoutItem* takeAt(int index);

    public:
        const QVBoxLayoutType* _baseType;
        ClassInstance* _obj;
        const CallEnvironment* _env;
    };

    inline ClassInstance*
    QVBoxLayoutType::cachedInstance(const QVBoxLayoutType::MuQtType* obj)
    {
        return obj->_obj;
    }

} // namespace Mu

#endif // __MuQt__QVBoxLayoutType__h__
