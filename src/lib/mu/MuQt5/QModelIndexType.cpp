//
// Copyright (c) 2009, Jim Hourihan
// All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
//
#include <MuQt5/qtUtils.h>
#include <MuQt5/QModelIndexType.h>
#include <MuQt5/QActionType.h>
#include <MuQt5/QWidgetType.h>
#include <Mu/Alias.h>
#include <Mu/BaseFunctions.h>
#include <Mu/ClassInstance.h>
#include <Mu/Exception.h>
#include <Mu/Function.h>
#include <Mu/MemberFunction.h>
#include <Mu/MemberVariable.h>
#include <Mu/Node.h>
#include <Mu/ParameterVariable.h>
#include <Mu/ReferenceType.h>
#include <Mu/SymbolicConstant.h>
#include <Mu/Thread.h>
#include <Mu/Value.h>
#include <MuLang/MuLangContext.h>
#include <MuLang/StringType.h>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>
#include <QtSvg/QtSvg>
#include <QtNetwork/QtNetwork>
#include <MuQt5/QAbstractItemModelType.h>
#include <MuQt5/QVariantType.h>

//
//  NOTE: this file was automatically generated by qt2mu.py
//

namespace Mu
{
    using namespace std;

    QModelIndexType::Instance::Instance(const Class* c)
        : ClassInstance(c)
    {
    }

    QModelIndexType::QModelIndexType(Context* c, const char* name, Class* super)
        : Class(c, name, super)
    {
    }

    QModelIndexType::~QModelIndexType() {}

    static NODE_IMPLEMENTATION(__allocate, Pointer)
    {
        QModelIndexType::Instance* i =
            new QModelIndexType::Instance((Class*)NODE_THIS.type());
        QModelIndexType::registerFinalizer(i);
        NODE_RETURN(i);
    }

    void QModelIndexType::registerFinalizer(void* o)
    {
        GC_register_finalizer(o, QModelIndexType::finalizer, 0, 0, 0);
    }

    void QModelIndexType::finalizer(void* obj, void* data)
    {
        QModelIndexType::Instance* i =
            reinterpret_cast<QModelIndexType::Instance*>(obj);
        delete i;
    }

    //----------------------------------------------------------------------
    //  PRE-COMPILED FUNCTIONS

    Pointer
    qt_QModelIndex_QModelIndex_QModelIndex_QModelIndex(Mu::Thread& NODE_THREAD,
                                                       Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        setqtype<QModelIndexType>(param_this, QModelIndex());
        return param_this;
    }

    Pointer qt_QModelIndex_child_QModelIndex_QModelIndex_int_int(
        Mu::Thread& NODE_THREAD, Pointer param_this, int param_row,
        int param_column)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QModelIndex arg0 = getqtype<QModelIndexType>(param_this);
        int arg1 = (int)(param_row);
        int arg2 = (int)(param_column);
        return makeqtype<QModelIndexType>(c, arg0.child(arg1, arg2),
                                          "qt.QModelIndex");
    }

    int qt_QModelIndex_column_int_QModelIndex(Mu::Thread& NODE_THREAD,
                                              Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QModelIndex arg0 = getqtype<QModelIndexType>(param_this);
        return arg0.column();
    }

    Pointer qt_QModelIndex_data_QVariant_QModelIndex_int(
        Mu::Thread& NODE_THREAD, Pointer param_this, int param_role)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QModelIndex arg0 = getqtype<QModelIndexType>(param_this);
        int arg1 = (int)(param_role);
        return makeqtype<QVariantType>(c, arg0.data(arg1), "qt.QVariant");
    }

    int qt_QModelIndex_flags_int_QModelIndex(Mu::Thread& NODE_THREAD,
                                             Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QModelIndex arg0 = getqtype<QModelIndexType>(param_this);
        return int(arg0.flags());
    }

    bool qt_QModelIndex_isValid_bool_QModelIndex(Mu::Thread& NODE_THREAD,
                                                 Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QModelIndex arg0 = getqtype<QModelIndexType>(param_this);
        return arg0.isValid();
    }

    Pointer
    qt_QModelIndex_model_QAbstractItemModel_QModelIndex(Mu::Thread& NODE_THREAD,
                                                        Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QModelIndex arg0 = getqtype<QModelIndexType>(param_this);
        return makeinstance<QAbstractItemModelType>(c, arg0.model(),
                                                    "qt.QAbstractItemModel");
    }

    Pointer
    qt_QModelIndex_parent_QModelIndex_QModelIndex(Mu::Thread& NODE_THREAD,
                                                  Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QModelIndex arg0 = getqtype<QModelIndexType>(param_this);
        return makeqtype<QModelIndexType>(c, arg0.parent(), "qt.QModelIndex");
    }

    int qt_QModelIndex_row_int_QModelIndex(Mu::Thread& NODE_THREAD,
                                           Pointer param_this)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QModelIndex arg0 = getqtype<QModelIndexType>(param_this);
        return arg0.row();
    }

    Pointer qt_QModelIndex_sibling_QModelIndex_QModelIndex_int_int(
        Mu::Thread& NODE_THREAD, Pointer param_this, int param_row,
        int param_column)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QModelIndex arg0 = getqtype<QModelIndexType>(param_this);
        int arg1 = (int)(param_row);
        int arg2 = (int)(param_column);
        return makeqtype<QModelIndexType>(c, arg0.sibling(arg1, arg2),
                                          "qt.QModelIndex");
    }

    bool qt_QModelIndex_operatorBang_EQ__bool_QModelIndex_QModelIndex(
        Mu::Thread& NODE_THREAD, Pointer param_this, Pointer param_other)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QModelIndex arg0 = getqtype<QModelIndexType>(param_this);
        const QModelIndex arg1 = getqtype<QModelIndexType>(param_other);
        return arg0.operator!=(arg1);
    }

    bool qt_QModelIndex_operatorLT__bool_QModelIndex_QModelIndex(
        Mu::Thread& NODE_THREAD, Pointer param_this, Pointer param_other)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QModelIndex arg0 = getqtype<QModelIndexType>(param_this);
        const QModelIndex arg1 = getqtype<QModelIndexType>(param_other);
        return arg0.operator<(arg1);
    }

    bool qt_QModelIndex_operatorEQ_EQ__bool_QModelIndex_QModelIndex(
        Mu::Thread& NODE_THREAD, Pointer param_this, Pointer param_other)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QModelIndex arg0 = getqtype<QModelIndexType>(param_this);
        const QModelIndex arg1 = getqtype<QModelIndexType>(param_other);
        return arg0.operator==(arg1);
    }

    static NODE_IMPLEMENTATION(_n_QModelIndex0, Pointer)
    {
        NODE_RETURN(qt_QModelIndex_QModelIndex_QModelIndex_QModelIndex(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_child0, Pointer)
    {
        NODE_RETURN(qt_QModelIndex_child_QModelIndex_QModelIndex_int_int(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, int),
            NODE_ARG(2, int)));
    }

    static NODE_IMPLEMENTATION(_n_column0, int)
    {
        NODE_RETURN(qt_QModelIndex_column_int_QModelIndex(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_data0, Pointer)
    {
        NODE_RETURN(qt_QModelIndex_data_QVariant_QModelIndex_int(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, int)));
    }

    static NODE_IMPLEMENTATION(_n_flags0, int)
    {
        NODE_RETURN(qt_QModelIndex_flags_int_QModelIndex(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_isValid0, bool)
    {
        NODE_RETURN(qt_QModelIndex_isValid_bool_QModelIndex(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_model0, Pointer)
    {
        NODE_RETURN(qt_QModelIndex_model_QAbstractItemModel_QModelIndex(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_parent0, Pointer)
    {
        NODE_RETURN(qt_QModelIndex_parent_QModelIndex_QModelIndex(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_row0, int)
    {
        NODE_RETURN(qt_QModelIndex_row_int_QModelIndex(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_sibling0, Pointer)
    {
        NODE_RETURN(qt_QModelIndex_sibling_QModelIndex_QModelIndex_int_int(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, int),
            NODE_ARG(2, int)));
    }

    static NODE_IMPLEMENTATION(_n_operatorBang_EQ_0, bool)
    {
        NODE_RETURN(
            qt_QModelIndex_operatorBang_EQ__bool_QModelIndex_QModelIndex(
                NODE_THREAD, NONNIL_NODE_ARG(0, Pointer),
                NODE_ARG(1, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_operatorLT_0, bool)
    {
        NODE_RETURN(qt_QModelIndex_operatorLT__bool_QModelIndex_QModelIndex(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_operatorEQ_EQ_0, bool)
    {
        NODE_RETURN(qt_QModelIndex_operatorEQ_EQ__bool_QModelIndex_QModelIndex(
            NODE_THREAD, NONNIL_NODE_ARG(0, Pointer), NODE_ARG(1, Pointer)));
    }

    static NODE_IMPLEMENTATION(_n_internalPointer, Pointer)
    {
        MuLangContext* c = static_cast<MuLangContext*>(NODE_THREAD.context());
        QModelIndex arg0 = getqtype<QModelIndexType>(NODE_ARG(0, Pointer));
        NODE_RETURN(Pointer(arg0.internalPointer()));
    }

    void QModelIndexType::load()
    {
        USING_MU_FUNCTION_SYMBOLS;
        MuLangContext* c = static_cast<MuLangContext*>(context());
        Module* global = globalModule();

        const string typeName = name();
        const string refTypeName = typeName + "&";
        const string fullTypeName = fullyQualifiedName();
        const string fullRefTypeName = fullTypeName + "&";
        const char* tn = typeName.c_str();
        const char* ftn = fullTypeName.c_str();
        const char* rtn = refTypeName.c_str();
        const char* frtn = fullRefTypeName.c_str();

        scope()->addSymbols(new ReferenceType(c, rtn, this),

                            new Function(c, tn, BaseFunctions::dereference,
                                         Cast, Return, ftn, Args, frtn, End),

                            EndArguments);

        addSymbols(
            new Function(c, "__allocate", __allocate, None, Return, ftn, End),

            EndArguments);

        addSymbols(EndArguments);

        addSymbols(
            // enums
            // member functions
            new Function(c, "QModelIndex", _n_QModelIndex0, None, Compiled,
                         qt_QModelIndex_QModelIndex_QModelIndex_QModelIndex,
                         Return, "qt.QModelIndex", Parameters,
                         new Param(c, "this", "qt.QModelIndex"), End),
            new Function(c, "child", _n_child0, None, Compiled,
                         qt_QModelIndex_child_QModelIndex_QModelIndex_int_int,
                         Return, "qt.QModelIndex", Parameters,
                         new Param(c, "this", "qt.QModelIndex"),
                         new Param(c, "row", "int"),
                         new Param(c, "column", "int"), End),
            new Function(c, "column", _n_column0, None, Compiled,
                         qt_QModelIndex_column_int_QModelIndex, Return, "int",
                         Parameters, new Param(c, "this", "qt.QModelIndex"),
                         End),
            new Function(
                c, "data", _n_data0, None, Compiled,
                qt_QModelIndex_data_QVariant_QModelIndex_int, Return,
                "qt.QVariant", Parameters,
                new Param(c, "this", "qt.QModelIndex"),
                new Param(c, "role", "int", Value((int)Qt::DisplayRole)), End),
            new Function(c, "flags", _n_flags0, None, Compiled,
                         qt_QModelIndex_flags_int_QModelIndex, Return, "int",
                         Parameters, new Param(c, "this", "qt.QModelIndex"),
                         End),
            // MISSING: internalId ("quintptr"; QModelIndex this)
            // MISSING: internalPointer ("void *"; QModelIndex this)
            new Function(c, "isValid", _n_isValid0, None, Compiled,
                         qt_QModelIndex_isValid_bool_QModelIndex, Return,
                         "bool", Parameters,
                         new Param(c, "this", "qt.QModelIndex"), End),
            new Function(c, "model", _n_model0, None, Compiled,
                         qt_QModelIndex_model_QAbstractItemModel_QModelIndex,
                         Return, "qt.QAbstractItemModel", Parameters,
                         new Param(c, "this", "qt.QModelIndex"), End),
            new Function(c, "parent", _n_parent0, None, Compiled,
                         qt_QModelIndex_parent_QModelIndex_QModelIndex, Return,
                         "qt.QModelIndex", Parameters,
                         new Param(c, "this", "qt.QModelIndex"), End),
            new Function(c, "row", _n_row0, None, Compiled,
                         qt_QModelIndex_row_int_QModelIndex, Return, "int",
                         Parameters, new Param(c, "this", "qt.QModelIndex"),
                         End),
            new Function(c, "sibling", _n_sibling0, None, Compiled,
                         qt_QModelIndex_sibling_QModelIndex_QModelIndex_int_int,
                         Return, "qt.QModelIndex", Parameters,
                         new Param(c, "this", "qt.QModelIndex"),
                         new Param(c, "row", "int"),
                         new Param(c, "column", "int"), End),
            // static functions
            EndArguments);
        globalScope()->addSymbols(
            new Function(
                c, "!=", _n_operatorBang_EQ_0, Op, Compiled,
                qt_QModelIndex_operatorBang_EQ__bool_QModelIndex_QModelIndex,
                Return, "bool", Parameters,
                new Param(c, "this", "qt.QModelIndex"),
                new Param(c, "other", "qt.QModelIndex"), End),
            new Function(
                c, "<", _n_operatorLT_0, Op, Compiled,
                qt_QModelIndex_operatorLT__bool_QModelIndex_QModelIndex, Return,
                "bool", Parameters, new Param(c, "this", "qt.QModelIndex"),
                new Param(c, "other", "qt.QModelIndex"), End),
            new Function(
                c, "==", _n_operatorEQ_EQ_0, Op, Compiled,
                qt_QModelIndex_operatorEQ_EQ__bool_QModelIndex_QModelIndex,
                Return, "bool", Parameters,
                new Param(c, "this", "qt.QModelIndex"),
                new Param(c, "other", "qt.QModelIndex"), End),
            EndArguments);
        scope()->addSymbols(EndArguments);

        c->arrayType(this, 1, 0);

        addSymbols(

            new Function(c, "internalPointer", _n_internalPointer, None, Return,
                         "object", Parameters,
                         new Param(c, "this", "qt.QModelIndex"), End),

            EndArguments);
    }

} // namespace Mu
