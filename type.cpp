/*
  Copyright (c) 2010-2012, Intel Corporation
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Intel Corporation nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.


   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
*/

/** @file type.cpp
    @brief Definitions for classes related to type representation
*/

#include "type.h"
#include "expr.h"
#include "sym.h"
#include "llvmutil.h"
#include "module.h"

#include <stdio.h>
#include <llvm/Value.h>
#include <llvm/Module.h>
#include <llvm/Analysis/DIBuilder.h>
#include <llvm/Analysis/DebugInfo.h>
#include <llvm/Support/Dwarf.h>


/** Utility routine used in code that prints out declarations; returns true
    if the given name should be printed, false otherwise.  This allows us
    to omit the names for various internal things (whose names start with
    double underscores) and emit anonymous declarations for them instead.
 */

static bool
lShouldPrintName(const std::string &name) {
    if (name.size() == 0)
        return false;
    else if (name[0] != '_')
        return true;
    else
        return (name.size() == 1) || (name[1] != '_');
}


/** Utility routine to create a llvm DIArray type of the given number of
    the given element type. */
static llvm::DIType 
lCreateDIArray(llvm::DIType eltType, int count) {
    int lowerBound = 0, upperBound = count-1;

    if (count == 0) {
        // unsized array -> indicate with low > high
        lowerBound = 1;
        upperBound = 0;
    }

    llvm::Value *sub = m->diBuilder->getOrCreateSubrange(lowerBound, upperBound);
    std::vector<llvm::Value *> subs;
    subs.push_back(sub);
#ifdef LLVM_2_9
    llvm::DIArray subArray = m->diBuilder->getOrCreateArray(&subs[0], subs.size());
#else
    llvm::DIArray subArray = m->diBuilder->getOrCreateArray(subs);
#endif

    uint64_t size = eltType.getSizeInBits() * count;
    uint64_t align = eltType.getAlignInBits();

    return m->diBuilder->createArrayType(size, align, eltType, subArray);
}


///////////////////////////////////////////////////////////////////////////
// Variability

std::string
Variability::GetString() const {
    switch (type) {
    case Uniform: return "uniform";
    case Varying: return "varying";
    case SOA: {
        char buf[32];
        sprintf(buf, "soa<%d>", soaWidth);
        return buf;
    }
    case Unbound: return "/*unbound*/";
    default:
        FATAL("Unhandled variability");
        return "";
    }
}


std::string
Variability::MangleString() const {
    switch (type) {
    case Uniform: 
        return "un";
    case Varying: 
        return "vy";
    case SOA: {
        char buf[32];
        sprintf(buf, "soa<%d>", soaWidth);
        return buf;
    }
    case Unbound:
        FATAL("Unbound unexpected in Variability::MangleString()");
    default:
        FATAL("Unhandled variability");
        return "";
    }
}

///////////////////////////////////////////////////////////////////////////
// AtomicType

const AtomicType *AtomicType::UniformBool =
    new AtomicType(AtomicType::TYPE_BOOL, Variability::Uniform, false);
const AtomicType *AtomicType::VaryingBool =
    new AtomicType(AtomicType::TYPE_BOOL, Variability::Varying, false);
const AtomicType *AtomicType::UniformInt8 =
    new AtomicType(AtomicType::TYPE_INT8, Variability::Uniform, false);
const AtomicType *AtomicType::VaryingInt8 =
    new AtomicType(AtomicType::TYPE_INT8, Variability::Varying, false);
const AtomicType *AtomicType::UniformUInt8 =
    new AtomicType(AtomicType::TYPE_UINT8, Variability::Uniform, false);
const AtomicType *AtomicType::VaryingUInt8 =
    new AtomicType(AtomicType::TYPE_UINT8, Variability::Varying, false);
const AtomicType *AtomicType::UniformInt16 =
    new AtomicType(AtomicType::TYPE_INT16, Variability::Uniform, false);
const AtomicType *AtomicType::VaryingInt16 =
    new AtomicType(AtomicType::TYPE_INT16, Variability::Varying, false);
const AtomicType *AtomicType::UniformUInt16 =
    new AtomicType(AtomicType::TYPE_UINT16, Variability::Uniform, false);
const AtomicType *AtomicType::VaryingUInt16 =
    new AtomicType(AtomicType::TYPE_UINT16, Variability::Varying, false);
const AtomicType *AtomicType::UniformInt32 =
    new AtomicType(AtomicType::TYPE_INT32, Variability::Uniform, false);
const AtomicType *AtomicType::VaryingInt32 =
    new AtomicType(AtomicType::TYPE_INT32, Variability::Varying, false);
const AtomicType *AtomicType::UniformUInt32 =
    new AtomicType(AtomicType::TYPE_UINT32, Variability::Uniform, false);
const AtomicType *AtomicType::VaryingUInt32 =
    new AtomicType(AtomicType::TYPE_UINT32, Variability::Varying, false);
const AtomicType *AtomicType::UniformFloat =
    new AtomicType(AtomicType::TYPE_FLOAT, Variability::Uniform, false);
const AtomicType *AtomicType::VaryingFloat =
    new AtomicType(AtomicType::TYPE_FLOAT, Variability::Varying, false);
const AtomicType *AtomicType::UniformInt64 =
    new AtomicType(AtomicType::TYPE_INT64, Variability::Uniform, false);
const AtomicType *AtomicType::VaryingInt64 =
    new AtomicType(AtomicType::TYPE_INT64, Variability::Varying, false);
const AtomicType *AtomicType::UniformUInt64 =
    new AtomicType(AtomicType::TYPE_UINT64, Variability::Uniform, false);
const AtomicType *AtomicType::VaryingUInt64 =
    new AtomicType(AtomicType::TYPE_UINT64, Variability::Varying, false);
const AtomicType *AtomicType::UniformDouble =
    new AtomicType(AtomicType::TYPE_DOUBLE, Variability::Uniform, false);
const AtomicType *AtomicType::VaryingDouble =
    new AtomicType(AtomicType::TYPE_DOUBLE, Variability::Varying, false);
const AtomicType *AtomicType::Void = 
    new AtomicType(TYPE_VOID, Variability::Uniform, false);


AtomicType::AtomicType(BasicType bt, Variability v, bool ic) 
    : basicType(bt), variability(v), isConst(ic) {
}


Variability
AtomicType::GetVariability() const {
    return variability;
}


bool
AtomicType::IsFloatType() const {
    return (basicType == TYPE_FLOAT || basicType == TYPE_DOUBLE);
}


bool
AtomicType::IsIntType() const {
    return (basicType == TYPE_INT8  || basicType == TYPE_UINT8  ||
            basicType == TYPE_INT16 || basicType == TYPE_UINT16 ||
            basicType == TYPE_INT32 || basicType == TYPE_UINT32 ||
            basicType == TYPE_INT64 || basicType == TYPE_UINT64);
}


bool
AtomicType::IsUnsignedType() const {
    return (basicType == TYPE_UINT8  || basicType == TYPE_UINT16 ||
            basicType == TYPE_UINT32 || basicType == TYPE_UINT64);
}


bool
AtomicType::IsBoolType() const {
    return basicType == TYPE_BOOL;
}


bool
AtomicType::IsConstType() const { 
    return isConst; 
}


const AtomicType *
AtomicType::GetAsUnsignedType() const {
    if (IsUnsignedType() == true) 
        return this;

    if (IsIntType() == false)
        return NULL;

    switch (basicType) {
    case TYPE_INT8:
        return new AtomicType(TYPE_UINT8, variability, isConst);
    case TYPE_INT16:
        return new AtomicType(TYPE_UINT16, variability, isConst);
    case TYPE_INT32:
        return new AtomicType(TYPE_UINT32, variability, isConst);
    case TYPE_INT64:
        return new AtomicType(TYPE_UINT64, variability, isConst);
    default:
        FATAL("Unexpected basicType in GetAsUnsignedType()");
        return NULL;
    }
}


const AtomicType *
AtomicType::GetAsConstType() const {
    if (Type::Equal(this, AtomicType::Void) || isConst == true) 
        return this;
    
    return new AtomicType(basicType, variability, true);
}


const AtomicType *
AtomicType::GetAsNonConstType() const {
    if (Type::Equal(this, AtomicType::Void) || isConst == false) 
        return this;

    return new AtomicType(basicType, variability, false);
}


const AtomicType *
AtomicType::GetBaseType() const {
    return this;
}


const AtomicType *
AtomicType::GetAsVaryingType() const {
    Assert(Type::Equal(this, AtomicType::Void) == false);
    if (variability == Variability::Varying)
        return this;
    return new AtomicType(basicType, Variability::Varying, isConst);
}


const AtomicType *
AtomicType::GetAsUniformType() const {
    Assert(Type::Equal(this, AtomicType::Void) == false);
    if (variability == Variability::Uniform)
        return this;
    return new AtomicType(basicType, Variability::Uniform, isConst);
}


const AtomicType *
AtomicType::GetAsUnboundVariabilityType() const {
    Assert(Type::Equal(this, AtomicType::Void) == false);
    if (variability == Variability::Unbound)
        return this;
    return new AtomicType(basicType, Variability::Unbound, isConst);
}


const AtomicType *
AtomicType::GetAsSOAType(int width) const {
    Assert(this != AtomicType::Void);
    if (variability == Variability(Variability::SOA, width))
        return this;
    return new AtomicType(basicType, Variability(Variability::SOA, width), isConst);
}


const AtomicType *
AtomicType::ResolveUnboundVariability(Variability v) const {
    Assert(v != Variability::Unbound);
    if (variability != Variability::Unbound)
        return this;
    return new AtomicType(basicType, v, isConst);
}


std::string
AtomicType::GetString() const {
    std::string ret;
    if (basicType != TYPE_VOID) {
        if (isConst)   ret += "const ";
        ret += variability.GetString();
        ret += " ";
    }

    switch (basicType) {
    case TYPE_VOID:   ret += "void";            break;
    case TYPE_BOOL:   ret += "bool";            break;
    case TYPE_INT8:   ret += "int8";            break;
    case TYPE_UINT8:  ret += "unsigned int8";   break;
    case TYPE_INT16:  ret += "int16";           break;
    case TYPE_UINT16: ret += "unsigned int16";  break;
    case TYPE_INT32:  ret += "int32";           break;
    case TYPE_UINT32: ret += "unsigned int32";  break;
    case TYPE_FLOAT:  ret += "float";           break;
    case TYPE_INT64:  ret += "int64";           break;
    case TYPE_UINT64: ret += "unsigned int64";  break;
    case TYPE_DOUBLE: ret += "double";          break;
    default: FATAL("Logic error in AtomicType::GetString()");
    }
    return ret;
}


std::string
AtomicType::Mangle() const {
    std::string ret;
    if (isConst)   ret += "C";
    ret += variability.MangleString();

    switch (basicType) {
    case TYPE_VOID:   ret += "v"; break;
    case TYPE_BOOL:   ret += "b"; break;
    case TYPE_INT8:   ret += "t"; break;
    case TYPE_UINT8:  ret += "T"; break;
    case TYPE_INT16:  ret += "s"; break;
    case TYPE_UINT16: ret += "S"; break;
    case TYPE_INT32:  ret += "i"; break;
    case TYPE_UINT32: ret += "u"; break;
    case TYPE_FLOAT:  ret += "f"; break;
    case TYPE_INT64:  ret += "I"; break;
    case TYPE_UINT64: ret += "U"; break;
    case TYPE_DOUBLE: ret += "d"; break;
    default: FATAL("Logic error in AtomicType::Mangle()");
    }
    return ret;
}


std::string
AtomicType::GetCDeclaration(const std::string &name) const {
    std::string ret;
    if (variability != Variability::Uniform && 
        variability != Variability::SOA) {
        Assert(m->errorCount > 0);
        return ret;
    }
    if (isConst) ret += "const ";

    switch (basicType) {
    case TYPE_VOID:   ret += "void";     break;
    case TYPE_BOOL:   ret += "bool";     break;
    case TYPE_INT8:   ret += "int8_t";   break;
    case TYPE_UINT8:  ret += "uint8_t";  break;
    case TYPE_INT16:  ret += "int16_t";  break;
    case TYPE_UINT16: ret += "uint16_t"; break;
    case TYPE_INT32:  ret += "int32_t";  break;
    case TYPE_UINT32: ret += "uint32_t"; break;
    case TYPE_FLOAT:  ret += "float";    break;
    case TYPE_INT64:  ret += "int64_t";  break;
    case TYPE_UINT64: ret += "uint64_t"; break;
    case TYPE_DOUBLE: ret += "double";   break;
    default: FATAL("Logic error in AtomicType::GetCDeclaration()");
    }

    if (lShouldPrintName(name)) {
        ret += " ";
        ret += name;
    }

    if (variability == Variability::SOA) {
        char buf[32];
        sprintf(buf, "[%d]", variability.soaWidth);
        ret += buf;
    }

    return ret;
}


LLVM_TYPE_CONST llvm::Type *
AtomicType::LLVMType(llvm::LLVMContext *ctx) const {
    Assert(variability.type != Variability::Unbound);
    bool isUniform = (variability == Variability::Uniform);
    bool isVarying = (variability == Variability::Varying);

    if (isUniform || isVarying) {
        switch (basicType) {
        case TYPE_VOID:
            return llvm::Type::getVoidTy(*ctx);
        case TYPE_BOOL:
            return isUniform ? LLVMTypes::BoolType : LLVMTypes::BoolVectorType;
        case TYPE_INT8:
        case TYPE_UINT8:
            return isUniform ? LLVMTypes::Int8Type : LLVMTypes::Int8VectorType;
        case TYPE_INT16:
        case TYPE_UINT16:
            return isUniform ? LLVMTypes::Int16Type : LLVMTypes::Int16VectorType;
        case TYPE_INT32:
        case TYPE_UINT32:
            return isUniform ? LLVMTypes::Int32Type : LLVMTypes::Int32VectorType;
        case TYPE_FLOAT:
            return isUniform ? LLVMTypes::FloatType : LLVMTypes::FloatVectorType;
        case TYPE_INT64:
        case TYPE_UINT64:
            return isUniform ? LLVMTypes::Int64Type : LLVMTypes::Int64VectorType;
        case TYPE_DOUBLE:
            return isUniform ? LLVMTypes::DoubleType : LLVMTypes::DoubleVectorType;
        default:
            FATAL("logic error in AtomicType::LLVMType");
            return NULL;
        }
    }
    else {
        ArrayType at(GetAsUniformType(), variability.soaWidth);
        return at.LLVMType(ctx);
    }
}


llvm::DIType
AtomicType::GetDIType(llvm::DIDescriptor scope) const {
    Assert(variability.type != Variability::Unbound);

    if (variability.type == Variability::Uniform) {
        switch (basicType) {
        case TYPE_VOID:
            return llvm::DIType();
        case TYPE_BOOL:
            return m->diBuilder->createBasicType("bool", 32 /* size */, 32 /* align */,
                                                 llvm::dwarf::DW_ATE_unsigned);
            break;
        case TYPE_INT8:
            return m->diBuilder->createBasicType("int8", 8 /* size */, 8 /* align */,
                                                 llvm::dwarf::DW_ATE_signed);
            break;
        case TYPE_UINT8:
            return m->diBuilder->createBasicType("uint8", 8 /* size */, 8 /* align */,
                                                 llvm::dwarf::DW_ATE_unsigned);
            break;
        case TYPE_INT16:
            return m->diBuilder->createBasicType("int16", 16 /* size */, 16 /* align */,
                                                 llvm::dwarf::DW_ATE_signed);
            break;
        case TYPE_UINT16:
            return m->diBuilder->createBasicType("uint16", 16 /* size */, 16 /* align */,
                                                 llvm::dwarf::DW_ATE_unsigned);
            break;
        case TYPE_INT32:
            return m->diBuilder->createBasicType("int32", 32 /* size */, 32 /* align */,
                                                 llvm::dwarf::DW_ATE_signed);
            break;
        case TYPE_UINT32:
            return m->diBuilder->createBasicType("uint32", 32 /* size */, 32 /* align */,
                                                 llvm::dwarf::DW_ATE_unsigned);
            break;
        case TYPE_FLOAT:
            return m->diBuilder->createBasicType("float", 32 /* size */, 32 /* align */,
                                                 llvm::dwarf::DW_ATE_float);
            break;
        case TYPE_DOUBLE:
            return m->diBuilder->createBasicType("double", 64 /* size */, 64 /* align */,
                                                 llvm::dwarf::DW_ATE_float);
            break;
        case TYPE_INT64:
            return m->diBuilder->createBasicType("int64", 64 /* size */, 64 /* align */,
                                                 llvm::dwarf::DW_ATE_signed);
            break;
        case TYPE_UINT64:
            return m->diBuilder->createBasicType("uint64", 64 /* size */, 64 /* align */,
                                                 llvm::dwarf::DW_ATE_unsigned);
            break;
        default:
            FATAL("unhandled basic type in AtomicType::GetDIType()");
            return llvm::DIType();
        }
    }
    else if (variability == Variability::Varying) {
        llvm::DIType unifType = GetAsUniformType()->GetDIType(scope);
        llvm::Value *sub = m->diBuilder->getOrCreateSubrange(0, g->target.vectorWidth-1);
#ifdef LLVM_2_9
        llvm::Value *suba[] = { sub };
        llvm::DIArray subArray = m->diBuilder->getOrCreateArray(suba, 1);
#else
        llvm::DIArray subArray = m->diBuilder->getOrCreateArray(sub);
#endif // LLVM_2_9
        uint64_t size =  unifType.getSizeInBits()  * g->target.vectorWidth;
        uint64_t align = unifType.getAlignInBits() * g->target.vectorWidth;
        return m->diBuilder->createVectorType(size, align, unifType, subArray);
    }
    else {
        Assert(variability == Variability::SOA);
        ArrayType at(GetAsUniformType(), variability.soaWidth);
        return at.GetDIType(scope);
    }
}


///////////////////////////////////////////////////////////////////////////
// EnumType

EnumType::EnumType(SourcePos p) 
    : pos(p) {
    //    name = "/* (anonymous) */";
    isConst = false;
    variability = Variability(Variability::Unbound);
}


EnumType::EnumType(const char *n, SourcePos p) 
    : pos(p), name(n) {
    isConst = false;
    variability = Variability(Variability::Unbound);
}


Variability
EnumType::GetVariability() const {
    return variability;
}


bool 
EnumType::IsBoolType() const {
    return false;
}


bool 
EnumType::IsFloatType() const {
    return false;
}


bool 
EnumType::IsIntType() const {
    return true;
}


bool 
EnumType::IsUnsignedType() const {
    return true;
}


bool 
EnumType::IsConstType() const {
    return isConst;
}


const EnumType *
EnumType::GetBaseType() const {
    return this;
}


const EnumType *
EnumType::GetAsUniformType() const {
    if (IsUniformType())
        return this;
    else {
        EnumType *enumType = new EnumType(*this);
        enumType->variability = Variability::Uniform;
        return enumType;
    }
}


const EnumType *
EnumType::ResolveUnboundVariability(Variability v) const {
    if (variability != Variability::Unbound)
        return this;
    else {
        EnumType *enumType = new EnumType(*this);
        enumType->variability = v;
        return enumType;
    }
}


const EnumType *
EnumType::GetAsVaryingType() const {
    if (IsVaryingType())
        return this;
    else {
        EnumType *enumType = new EnumType(*this);
        enumType->variability = Variability(Variability::Varying);
        return enumType;
    }
}


const EnumType *
EnumType::GetAsUnboundVariabilityType() const {
    if (HasUnboundVariability())
        return this;
    else {
        EnumType *enumType = new EnumType(*this);
        enumType->variability = Variability(Variability::Unbound);
        return enumType;
    }
}


const EnumType *
EnumType::GetAsSOAType(int width) const {
    if (GetSOAWidth() == width)
        return this;
    else {
        EnumType *enumType = new EnumType(*this);
        enumType->variability = Variability(Variability::SOA, width);
        return enumType;
    }
}


const EnumType *
EnumType::GetAsConstType() const {
    if (isConst)
        return this;
    else {
        EnumType *enumType = new EnumType(*this);
        enumType->isConst = true;
        return enumType;
    }
}


const EnumType *
EnumType::GetAsNonConstType() const {
    if (!isConst)
        return this;
    else {
        EnumType *enumType = new EnumType(*this);
        enumType->isConst = false;
        return enumType;
    }
}


std::string 
EnumType::GetString() const {
    std::string ret;
    if (isConst) ret += "const ";
    ret += variability.GetString();

    ret += " enum ";
    if (name.size())
        ret += name;
    return ret;
}


std::string 
EnumType::Mangle() const {
    Assert(variability != Variability::Unbound);

    std::string ret;
    if (isConst) ret += "C";
    ret += variability.MangleString();
    ret += std::string("enum[") + name + std::string("]");
    return ret;
}


std::string 
EnumType::GetCDeclaration(const std::string &varName) const {
    if (variability != Variability::Uniform &&
        variability != Variability::SOA) {
        Assert(m->errorCount > 0);
        return "";
    }

    std::string ret;
    if (isConst) ret += "const ";
    ret += "enum";
    if (name.size())
        ret += std::string(" ") + name;

    if (lShouldPrintName(varName)) {
        ret += " ";
        ret += varName;
    }

    if (variability == Variability::SOA) {
        char buf[32];
        sprintf(buf, "[%d]", variability.soaWidth);
        ret += buf;
    }

    return ret;
}


LLVM_TYPE_CONST llvm::Type *
EnumType::LLVMType(llvm::LLVMContext *ctx) const {
    Assert(variability != Variability::Unbound);

    switch (variability.type) {
    case Variability::Uniform:
        return LLVMTypes::Int32Type;
    case Variability::Varying:
        return LLVMTypes::Int32VectorType;
    case Variability::SOA: {
        ArrayType at(AtomicType::UniformInt32, variability.soaWidth);
        return at.LLVMType(ctx);
    }
    default:
        FATAL("Unexpected variability in EnumType::LLVMType()");
        return NULL;
    }
}


llvm::DIType 
EnumType::GetDIType(llvm::DIDescriptor scope) const {
    std::vector<llvm::Value *> enumeratorDescriptors;
    for (unsigned int i = 0; i < enumerators.size(); ++i) {
        unsigned int enumeratorValue;
        Assert(enumerators[i]->constValue != NULL);
        int count = enumerators[i]->constValue->AsUInt32(&enumeratorValue);
        Assert(count == 1);

        llvm::Value *descriptor = 
            m->diBuilder->createEnumerator(enumerators[i]->name, enumeratorValue);
        enumeratorDescriptors.push_back(descriptor);
    }
#ifdef LLVM_2_9
    llvm::DIArray elementArray = 
        m->diBuilder->getOrCreateArray(&enumeratorDescriptors[0],
                                       enumeratorDescriptors.size());
#else
    llvm::DIArray elementArray = 
        m->diBuilder->getOrCreateArray(enumeratorDescriptors);
#endif

    llvm::DIFile diFile = pos.GetDIFile();
    llvm::DIType diType =
        m->diBuilder->createEnumerationType(scope, name, diFile, pos.first_line,
                                            32 /* size in bits */,
                                            32 /* align in bits */,
                                            elementArray);


    switch (variability.type) {
    case Variability::Uniform:
        return diType;
    case Variability::Varying: {
        llvm::Value *sub = m->diBuilder->getOrCreateSubrange(0, g->target.vectorWidth-1);
#ifdef LLVM_2_9
        llvm::Value *suba[] = { sub };
        llvm::DIArray subArray = m->diBuilder->getOrCreateArray(suba, 1);
#else
        llvm::DIArray subArray = m->diBuilder->getOrCreateArray(sub);
#endif // !LLVM_2_9
        uint64_t size =  diType.getSizeInBits()  * g->target.vectorWidth;
        uint64_t align = diType.getAlignInBits() * g->target.vectorWidth;
        return m->diBuilder->createVectorType(size, align, diType, subArray);
    }
    case Variability::SOA: {
        return lCreateDIArray(diType, variability.soaWidth);
    }
    default:
        FATAL("Unexpected variability in EnumType::GetDIType()");
        return llvm::DIType();
    }
}


void
EnumType::SetEnumerators(const std::vector<Symbol *> &e) {
    enumerators = e;
}


int
EnumType::GetEnumeratorCount() const {
    return (int)enumerators.size();
}


const Symbol *
EnumType::GetEnumerator(int i) const {
    return enumerators[i];
}


///////////////////////////////////////////////////////////////////////////
// PointerType

PointerType *PointerType::Void = 
    new PointerType(AtomicType::Void, Variability(Variability::Uniform), false);


PointerType::PointerType(const Type *t, Variability v, bool ic, bool is, 
                         bool fr)
    : variability(v), isConst(ic), isSlice(is), isFrozen(fr) {
    baseType = t;
}


PointerType *
PointerType::GetUniform(const Type *t) {
    return new PointerType(t, Variability(Variability::Uniform), false);
}


PointerType *
PointerType::GetVarying(const Type *t) {
    return new PointerType(t, Variability(Variability::Varying), false);
}


bool
PointerType::IsVoidPointer(const Type *t) {
    return Type::EqualIgnoringConst(t->GetAsUniformType(),
                                    PointerType::Void);
}


Variability
PointerType::GetVariability() const {
    return variability;
}


bool
PointerType::IsBoolType() const {
    return false;
}


bool
PointerType::IsFloatType() const {
    return false;
}


bool
PointerType::IsIntType() const {
    return false;
}


bool
PointerType::IsUnsignedType() const {
    return false;
}


bool
PointerType::IsConstType() const {
    return isConst;
}


const Type *
PointerType::GetBaseType() const {
    return baseType;
}


const PointerType *
PointerType::GetAsVaryingType() const {
    if (variability == Variability::Varying)
        return this;
    else
        return new PointerType(baseType, Variability(Variability::Varying),
                               isConst, isSlice, isFrozen);
}


const PointerType *
PointerType::GetAsUniformType() const {
    if (variability == Variability::Uniform)
        return this;
    else
        return new PointerType(baseType, Variability(Variability::Uniform),
                               isConst, isSlice, isFrozen);
}


const PointerType *
PointerType::GetAsUnboundVariabilityType() const {
    if (variability == Variability::Unbound)
        return this;
    else
        return new PointerType(baseType, Variability(Variability::Unbound),
                               isConst, isSlice, isFrozen);
}


const PointerType *
PointerType::GetAsSOAType(int width) const {
    if (GetSOAWidth() == width)
        return this;
    else
        return new PointerType(baseType, Variability(Variability::SOA, width),
                               isConst, isSlice, isFrozen);
}


const PointerType *
PointerType::GetAsSlice() const {
    if (isSlice)
        return this;
    return new PointerType(baseType, variability, isConst, true);
}


const PointerType *
PointerType::GetAsNonSlice() const {
    if (isSlice == false)
        return this;
    return new PointerType(baseType, variability, isConst, false);
}


const PointerType *
PointerType::GetAsFrozenSlice() const {
    if (isFrozen)
        return this;
    return new PointerType(baseType, variability, isConst, true, true);
}


/** Returns a structure corresponding to the pointer representation for
    slice pointers; the first member of this structure is a uniform or
    varying pointer, and the second element is either a uniform or varying
    int32.
 */
const StructType *
PointerType::GetSliceStructType() const {
    Assert(isSlice == true);

    std::vector<const Type *> eltTypes;
    eltTypes.push_back(GetAsNonSlice());
    switch (variability.type) {
    case Variability::Uniform:
        eltTypes.push_back(AtomicType::UniformInt32);
        break;
    case Variability::Varying:
        eltTypes.push_back(AtomicType::VaryingInt32);
        break;
    default:
        FATAL("Unexpected variability in PointerType::GetSliceStructType()");
    }

    std::vector<std::string> eltNames;
    std::vector<SourcePos> eltPos;

    eltNames.push_back("ptr");
    eltNames.push_back("offset");

    eltPos.push_back(SourcePos());
    eltPos.push_back(SourcePos());

    return new StructType("__ptr_slice_tmp", eltTypes, eltNames, eltPos, isConst,
                          Variability::Uniform, SourcePos());
}


const PointerType *
PointerType::ResolveUnboundVariability(Variability v) const {
    if (baseType == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }

    Assert(v != Variability::Unbound);
    Variability ptrVariability = (variability == Variability::Unbound) ? v :
        variability;
    const Type *resolvedBaseType = 
        baseType->ResolveUnboundVariability(Variability::Uniform);
    return new PointerType(resolvedBaseType, ptrVariability, isConst, isSlice,
                           isFrozen);
}


const PointerType *
PointerType::GetAsConstType() const {
    if (isConst == true)
        return this;
    else
        return new PointerType(baseType, variability, true, isSlice);
}


const PointerType *
PointerType::GetAsNonConstType() const {
    if (isConst == false)
        return this;
    else
        return new PointerType(baseType, variability, false, isSlice);
}


std::string 
PointerType::GetString() const {
    if (baseType == NULL) {
        Assert(m->errorCount > 0);
        return "";
    }

    std::string ret = baseType->GetString();

    ret += std::string(" * ");
    if (isConst) ret += "const ";
    if (isSlice) ret += "slice ";
    if (isFrozen) ret += "/*frozen*/ ";
    ret += variability.GetString();

    return ret;
}


std::string
PointerType::Mangle() const {
    Assert(variability != Variability::Unbound);
    if (baseType == NULL) {
        Assert(m->errorCount > 0);
        return "";
    }

    std::string ret = variability.MangleString() + std::string("<");
    if (isSlice || isFrozen)  ret += "-";
    if (isSlice) ret += "s";
    if (isFrozen) ret += "f";
    if (isSlice || isFrozen)  ret += "-";
    return ret + baseType->Mangle() + std::string(">");
}


std::string
PointerType::GetCDeclaration(const std::string &name) const {
    if (isSlice ||
        (variability != Variability::Uniform &&
         variability != Variability::SOA)) {
        Assert(m->errorCount > 0);
        return "";
    }

    if (baseType == NULL) {
        Assert(m->errorCount > 0);
        return "";
    }

    std::string ret = baseType->GetCDeclaration("");
    ret += std::string(" *");
    if (isConst) ret += " const";
    ret += std::string(" ");
    ret += name;

    if (variability == Variability::SOA) {
        char buf[32];
        sprintf(buf, "[%d]", variability.soaWidth);
        ret += buf;
    }

    return ret;
}


LLVM_TYPE_CONST llvm::Type *
PointerType::LLVMType(llvm::LLVMContext *ctx) const {
    if (baseType == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }

    if (isSlice)
        // Slice pointers are represented as a structure with a pointer and
        // an integer offset; the corresponding ispc type is returned by
        // GetSliceStructType().
        return GetSliceStructType()->LLVMType(ctx);

    switch (variability.type) {
    case Variability::Uniform: {
        LLVM_TYPE_CONST llvm::Type *ptype = NULL;
        const FunctionType *ftype = dynamic_cast<const FunctionType *>(baseType);
        if (ftype != NULL) 
            // Get the type of the function variant that takes the mask as the
            // last parameter--i.e. we don't allow taking function pointers of
            // exported functions.
            ptype = llvm::PointerType::get(ftype->LLVMFunctionType(ctx, true), 0);
        else {
            if (baseType == AtomicType::Void)
                ptype = LLVMTypes::VoidPointerType;
            else
                ptype = llvm::PointerType::get(baseType->LLVMType(ctx), 0);
        }
        return ptype;
    }
    case Variability::Varying:
        // always the same, since we currently use int vectors for varying
        // pointers
        return LLVMTypes::VoidPointerVectorType;
    case Variability::SOA: {
        ArrayType at(GetAsUniformType(), variability.soaWidth);
        return at.LLVMType(ctx);
    }
    default:
        FATAL("Unexpected variability in PointerType::LLVMType()");
        return NULL;
    }
}


llvm::DIType
PointerType::GetDIType(llvm::DIDescriptor scope) const {
    if (baseType == NULL) {
        Assert(m->errorCount > 0);
        return llvm::DIType();
    }

    llvm::DIType diTargetType = baseType->GetDIType(scope);
    int bitsSize = g->target.is32Bit ? 32 : 64;
    switch (variability.type) {
    case Variability::Uniform:
        return m->diBuilder->createPointerType(diTargetType, bitsSize);
    case Variability::Varying: {
        // emit them as an array of pointers
        llvm::DIType eltType = m->diBuilder->createPointerType(diTargetType, 
                                                               bitsSize);
        return lCreateDIArray(eltType, g->target.vectorWidth);
    }
    case Variability::SOA: {
        ArrayType at(GetAsUniformType(), variability.soaWidth);
        return at.GetDIType(scope);
    }
    default:
        FATAL("Unexpected variability in PointerType::GetDIType()");
        return llvm::DIType();
    }
}


///////////////////////////////////////////////////////////////////////////
// SequentialType

const Type *SequentialType::GetElementType(int index) const {
    return GetElementType();
}


///////////////////////////////////////////////////////////////////////////
// ArrayType

ArrayType::ArrayType(const Type *c, int a) 
    : child(c), numElements(a) {
    // 0 -> unsized array.
    Assert(numElements >= 0);
    Assert(Type::Equal(c, AtomicType::Void) == false);
}


LLVM_TYPE_CONST llvm::ArrayType *
ArrayType::LLVMType(llvm::LLVMContext *ctx) const {
    if (child == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }

    LLVM_TYPE_CONST llvm::Type *ct = child->LLVMType(ctx);
    if (ct == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }
    return llvm::ArrayType::get(ct, numElements);
}


Variability
ArrayType::GetVariability() const {
    return child ? child->GetVariability() : Variability(Variability::Uniform);
}


bool
ArrayType::IsFloatType() const {
    return false; 
}


bool
ArrayType::IsIntType() const {
    return false; 
}


bool
ArrayType::IsUnsignedType() const {
    return false; 
}


bool
ArrayType::IsBoolType() const {
    return false; 
}


bool
ArrayType::IsConstType() const {
    return child ? child->IsConstType() : false;
}


const Type *
ArrayType::GetBaseType() const {
    const Type *type = child;
    const ArrayType *at = dynamic_cast<const ArrayType *>(type);
    // Keep walking until we reach a child that isn't itself an array
    while (at) {
        type = at->child;
        at = dynamic_cast<const ArrayType *>(type);
    }
    return type;
}


const ArrayType *
ArrayType::GetAsVaryingType() const {
    if (child == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }
    return new ArrayType(child->GetAsVaryingType(), numElements);
}


const ArrayType *
ArrayType::GetAsUniformType() const {
    if (child == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }
    return new ArrayType(child->GetAsUniformType(), numElements);
}


const ArrayType *
ArrayType::GetAsUnboundVariabilityType() const {
    if (child == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }
    return new ArrayType(child->GetAsUnboundVariabilityType(), numElements);
}


const ArrayType *
ArrayType::GetAsSOAType(int width) const {
    if (child == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }
    return new ArrayType(child->GetAsSOAType(width), numElements);
}


const ArrayType *
ArrayType::ResolveUnboundVariability(Variability v) const {
    if (child == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }
    return new ArrayType(child->ResolveUnboundVariability(v), numElements);
}


const ArrayType *
ArrayType::GetAsUnsignedType() const {
    if (child == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }
    return new ArrayType(child->GetAsUnsignedType(), numElements);
}


const ArrayType *
ArrayType::GetAsConstType() const {
    if (child == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }
    return new ArrayType(child->GetAsConstType(), numElements);
}


const ArrayType *
ArrayType::GetAsNonConstType() const {
    if (child == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }
    return new ArrayType(child->GetAsNonConstType(), numElements);
}


int
ArrayType::GetElementCount() const {
    return numElements;
}


const Type *
ArrayType::GetElementType() const {
    return child;
}


std::string
ArrayType::GetString() const {
    const Type *base = GetBaseType();
    if (base == NULL) {
        Assert(m->errorCount > 0);
        return "";
    }
    std::string s = base->GetString();

    const ArrayType *at = this;
    // Walk through this and any children arrays and print all of their
    // dimensions
    while (at) {
        char buf[16];
        if (at->numElements > 0)
            sprintf(buf, "%d", at->numElements);
        else
            buf[0] = '\0';
        s += std::string("[") + std::string(buf) + std::string("]");
        at = dynamic_cast<const ArrayType *>(at->child);
    }
    return s;
}


std::string
ArrayType::Mangle() const {
    if (child == NULL) {
        Assert(m->errorCount > 0);
        return "(error)";
    }
    std::string s = child->Mangle();
    char buf[16];
    if (numElements > 0)
        sprintf(buf, "%d", numElements);
    else
        buf[0] = '\0';
    return s + "[" + buf + "]";
}


std::string
ArrayType::GetCDeclaration(const std::string &name) const {
    const Type *base = GetBaseType();
    if (base == NULL) {
        Assert(m->errorCount > 0);
        return "";
    }

    int soaWidth = base->GetSOAWidth();
    base = base->GetAsUniformType();

    std::string s = base->GetCDeclaration(name);

    const ArrayType *at = this;
    while (at) {
        char buf[16];
        if (at->numElements > 0)
            sprintf(buf, "%d", at->numElements);
        else
            buf[0] = '\0';
        s += std::string("[") + std::string(buf) + std::string("]");
        at = dynamic_cast<const ArrayType *>(at->child);
    }

    if (soaWidth > 0) {
        char buf[16];
        sprintf(buf, "[%d]", soaWidth);
        s += buf;
    }

    return s;
}


int
ArrayType::TotalElementCount() const {
    const ArrayType *ct = dynamic_cast<const ArrayType *>(child);
    if (ct != NULL)
        return numElements * ct->TotalElementCount();
    else
        return numElements;
}


llvm::DIType
ArrayType::GetDIType(llvm::DIDescriptor scope) const {
    if (child == NULL) {
        Assert(m->errorCount > 0);
        return llvm::DIType();
    }

    llvm::DIType eltType = child->GetDIType(scope);
    return lCreateDIArray(eltType, numElements);
}


ArrayType *
ArrayType::GetSizedArray(int sz) const {
    Assert(numElements == 0);
    return new ArrayType(child, sz);
}


const Type *
ArrayType::SizeUnsizedArrays(const Type *type, Expr *initExpr) {
    const ArrayType *at = dynamic_cast<const ArrayType *>(type);
    if (at == NULL)
        return type;

    ExprList *exprList = dynamic_cast<ExprList *>(initExpr);
    if (exprList == NULL || exprList->exprs.size() == 0)
        return type;

    // If the current dimension is unsized, then size it according to the
    // length of the expression list
    if (at->GetElementCount() == 0) {
        type = at->GetSizedArray(exprList->exprs.size());
        at = dynamic_cast<const ArrayType *>(type);
    }

    // Is there another nested level of expression lists?  If not, bail out
    // now.  Otherwise we'll use the first one to size the next dimension
    // (after checking below that it has the same length as all of the
    // other ones.
    ExprList *nextList = dynamic_cast<ExprList *>(exprList->exprs[0]);
    if (nextList == NULL)
        return type;

    const Type *nextType = at->GetElementType();
    const ArrayType *nextArrayType = dynamic_cast<const ArrayType *>(nextType);
    if (nextArrayType != NULL && nextArrayType->GetElementCount() == 0) {
        // If the recursive call to SizeUnsizedArrays at the bottom of the
        // function is going to size an unsized dimension, make sure that
        // all of the sub-expression lists are the same length--i.e. issue
        // an error if we have something like
        // int x[][] = { { 1 }, { 1, 2, 3, 4 } };
        unsigned int nextSize = nextList->exprs.size();
        for (unsigned int i = 1; i < exprList->exprs.size(); ++i) {
            if (exprList->exprs[i] == NULL) {
                // We should have seen an error earlier in this case.
                Assert(m->errorCount > 0);
                continue;
            }

            ExprList *el = dynamic_cast<ExprList *>(exprList->exprs[i]);
            if (el == NULL || el->exprs.size() != nextSize) {
                Error(Union(exprList->exprs[0]->pos, exprList->exprs[i]->pos), 
                      "Inconsistent initializer expression list lengths "
                      "make it impossible to size unsized array dimensions.");
                return NULL;
            }
        }
    }

    // Recursively call SizeUnsizedArrays() to get the child type for the
    // array that we were able to size here.
    return new ArrayType(SizeUnsizedArrays(at->GetElementType(), nextList),
                         at->GetElementCount());
}


///////////////////////////////////////////////////////////////////////////
// VectorType

VectorType::VectorType(const AtomicType *b, int a) 
    : base(b), numElements(a) {
    Assert(numElements > 0);
    Assert(base != NULL);
}


Variability
VectorType::GetVariability() const {
    return base->GetVariability(); 
}


bool
VectorType::IsFloatType() const {
    return base->IsFloatType(); 
}


bool
VectorType::IsIntType() const {
    return base->IsIntType(); 
}


bool
VectorType::IsUnsignedType() const {
    return base->IsUnsignedType(); 
}


bool
VectorType::IsBoolType() const {
    return base->IsBoolType(); 
}


bool
VectorType::IsConstType() const {
    return base->IsConstType(); 
}


const Type *
VectorType::GetBaseType() const {
    return base;
}


const VectorType *
VectorType::GetAsVaryingType() const {
    return new VectorType(base->GetAsVaryingType(), numElements);
}


const VectorType *
VectorType::GetAsUniformType() const {
    return new VectorType(base->GetAsUniformType(), numElements);
}


const VectorType *
VectorType::GetAsUnboundVariabilityType() const {
    return new VectorType(base->GetAsUnboundVariabilityType(), numElements);
}


const VectorType *
VectorType::GetAsSOAType(int width) const {
    return new VectorType(base->GetAsSOAType(width), numElements);
}


const VectorType *
VectorType::ResolveUnboundVariability(Variability v) const {
    return new VectorType(base->ResolveUnboundVariability(v), numElements);
}


const VectorType *
VectorType::GetAsConstType() const {
    return new VectorType(base->GetAsConstType(), numElements);
}


const VectorType *
VectorType::GetAsNonConstType() const {
    return new VectorType(base->GetAsNonConstType(), numElements);
}


std::string
VectorType::GetString() const {
    std::string s = base->GetString();
    char buf[16];
    sprintf(buf, "<%d>", numElements);
    return s + std::string(buf);
}


std::string
VectorType::Mangle() const {
    std::string s = base->Mangle();
    char buf[16];
    sprintf(buf, "<%d>", numElements);
    return s + std::string(buf);
}


std::string
VectorType::GetCDeclaration(const std::string &name) const {
    std::string s = base->GetCDeclaration("");
    char buf[16];
    sprintf(buf, "%d", numElements);
    return s + std::string(buf) + "  " + name;
}


int
VectorType::GetElementCount() const {
    return numElements;
}


const AtomicType *
VectorType::GetElementType() const {
    return base;
}


LLVM_TYPE_CONST llvm::Type *
VectorType::LLVMType(llvm::LLVMContext *ctx) const {
    if (base == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }

    LLVM_TYPE_CONST llvm::Type *bt = base->LLVMType(ctx);
    if (!bt)
        return NULL;

    if (base->IsUniformType())
        // vectors of uniform types are laid out across LLVM vectors, with
        // the llvm vector size set to be a multiple of the machine's
        // natural vector size (e.g. 4 on SSE).  This is a roundabout way
        // of ensuring that LLVM lays them out into machine vector
        // registers so that e.g. if we want to add two uniform 4 float
        // vectors, that is turned into a single addps on SSE.
        return llvm::VectorType::get(bt, getVectorMemoryCount());
    else if (base->IsVaryingType())
        // varying types are already laid out to fill HW vector registers,
        // so a vector type here is just expanded out as an llvm array.
        return llvm::ArrayType::get(bt, getVectorMemoryCount());
    else if (base->IsSOAType())
        return llvm::ArrayType::get(bt, numElements);
    else {
        FATAL("Unexpected variability in VectorType::LLVMType()");
        return NULL;
    }
}


llvm::DIType
VectorType::GetDIType(llvm::DIDescriptor scope) const {
    llvm::DIType eltType = base->GetDIType(scope);
    llvm::Value *sub = m->diBuilder->getOrCreateSubrange(0, numElements-1);
#ifdef LLVM_2_9
    llvm::Value *subs[1] = { sub };
    llvm::DIArray subArray = m->diBuilder->getOrCreateArray(subs, 1);
#else
    llvm::DIArray subArray = m->diBuilder->getOrCreateArray(sub);
#endif

    uint64_t sizeBits = eltType.getSizeInBits() * numElements;

    // vectors of varying types are already naturally aligned to the
    // machine's vector width, but arrays of uniform types need to be
    // explicitly aligned to the machines natural vector alignment.
    uint64_t align = eltType.getAlignInBits();
    if (IsUniformType())
        align = 4 * g->target.nativeVectorWidth;

    if (IsUniformType() || IsVaryingType())
        return m->diBuilder->createVectorType(sizeBits, align, eltType, subArray);
    else if (IsSOAType()) {
        ArrayType at(base, numElements);
        return at.GetDIType(scope);
    }
    else {
        FATAL("Unexpected variability in VectorType::GetDIType()");
        return llvm::DIType();
    }
}


int
VectorType::getVectorMemoryCount() const {
    if (base->IsVaryingType())
        return numElements;
    else if (base->IsUniformType()) {
        int nativeWidth = g->target.nativeVectorWidth;
        if (Type::Equal(base->GetAsUniformType(), AtomicType::UniformInt64) ||
            Type::Equal(base->GetAsUniformType(), AtomicType::UniformUInt64) ||
            Type::Equal(base->GetAsUniformType(), AtomicType::UniformDouble))
            // target.nativeVectorWidth should be in terms of 32-bit
            // values, so for the 64-bit guys, it takes half as many of
            // them to fill the native width
            nativeWidth /= 2;
        // and now round up the element count to be a multiple of
        // nativeWidth
        return (numElements + (nativeWidth - 1)) & ~(nativeWidth-1);
    }
    else if (base->IsSOAType()) {
        FATAL("VectorType SOA getVectorMemoryCount");
        return -1;
    }
    else {
        FATAL("Unexpected variability in VectorType::getVectorMemoryCount()");
        return -1;
    }
}


///////////////////////////////////////////////////////////////////////////
// StructType

StructType::StructType(const std::string &n, const std::vector<const Type *> &elts, 
                       const std::vector<std::string> &en,
                       const std::vector<SourcePos> &ep,
                       bool ic, Variability v, SourcePos p) 
    : name(n), elementTypes(elts), elementNames(en), elementPositions(ep),
      variability(v), isConst(ic), pos(p) {
}


Variability
StructType::GetVariability() const  {
    return variability; 
}


bool
StructType::IsBoolType() const {
    return false; 
}


bool
StructType::IsFloatType() const {
    return false; 
}


bool
StructType::IsIntType() const  {
    return false; 
}


bool
StructType::IsUnsignedType() const {
    return false; 
}


bool
StructType::IsConstType() const {
    return isConst; 
}


const Type *
StructType::GetBaseType() const {
    return this;
}


const StructType *
StructType::GetAsVaryingType() const {
    if (IsVaryingType()) 
        return this;
    else
        return new StructType(name, elementTypes, elementNames, elementPositions,
                              isConst, Variability(Variability::Varying), pos);
}


const StructType *
StructType::GetAsUniformType() const {
    if (IsUniformType()) 
        return this;
    else
        return new StructType(name, elementTypes, elementNames, elementPositions,
                              isConst, Variability(Variability::Uniform), pos);
}


const StructType *
StructType::GetAsUnboundVariabilityType() const {
    if (HasUnboundVariability()) 
        return this;
    else
        return new StructType(name, elementTypes, elementNames, elementPositions,
                              isConst, Variability(Variability::Unbound), pos);
}


const StructType *
StructType::GetAsSOAType(int width) const {
    if (GetSOAWidth() == width)
        return this;

    if (checkIfCanBeSOA(this) == false)
        return NULL;

    return new StructType(name, elementTypes, elementNames, elementPositions,
                          isConst, Variability(Variability::SOA, width), pos);
}


const StructType *
StructType::ResolveUnboundVariability(Variability v) const {
    Assert(v != Variability::Unbound);

    if (variability != Variability::Unbound)
        return this;

    // We don't resolve the members here but leave them unbound, so that if
    // resolve to varying but later want to get the uniform version of this
    // type, for example, then we still have the information around about
    // which element types were originally unbound...
    return new StructType(name, elementTypes, elementNames, elementPositions,
                          isConst, v, pos);
}


const StructType *
StructType::GetAsConstType() const {
    if (IsConstType()) 
        return this;
    else
        return new StructType(name, elementTypes, elementNames, elementPositions,
                              true, variability, pos);
}


const StructType *
StructType::GetAsNonConstType() const {
    if (!IsConstType()) 
        return this;
    else
        return new StructType(name, elementTypes, elementNames, elementPositions,
                              false, variability, pos);
}


std::string
StructType::GetString() const {
    std::string ret;
    if (isConst)   ret += "const ";
    ret += variability.GetString();
    ret += " ";

    // Don't print the entire struct declaration, just print the struct's name.
    // @todo Do we need a separate method that prints the declaration?
#if 0
    ret += std::string("struct { ") + name;
    for (unsigned int i = 0; i < elementTypes.size(); ++i) {
        ret += elementTypes[i]->GetString();
        ret += " ";
        ret += elementNames[i];
        ret += "; ";
    }
    ret += "}";
#else
    ret += "struct ";
    ret += name;
#endif
    return ret;
}


std::string
StructType::Mangle() const {
    Assert(variability != Variability::Unbound);

    std::string ret;
    ret += "s[";
    if (isConst)
        ret += "_c_";
    ret += variability.MangleString();

    ret += name + std::string("]<");
    for (unsigned int i = 0; i < elementTypes.size(); ++i)
        ret += GetElementType(i)->Mangle();
    ret += ">";
    return ret;
}
    

std::string
StructType::GetCDeclaration(const std::string &n) const {
    std::string ret;
    if (isConst) ret += "const ";
    ret += std::string("struct ") + name;
    if (lShouldPrintName(n))
        ret += std::string(" ") + n;

    if (variability.soaWidth > 0) {
        char buf[32];
        // This has to match the naming scheme used in lEmitStructDecls()
        // in module.cpp
        sprintf(buf, "_SOA%d", variability.soaWidth);
        ret += buf;
    }

    return ret;
}


LLVM_TYPE_CONST llvm::Type *
StructType::LLVMType(llvm::LLVMContext *ctx) const {
    std::vector<LLVM_TYPE_CONST llvm::Type *> llvmTypes;
    for (int i = 0; i < GetElementCount(); ++i) {
        const Type *type = GetElementType(i);
        if (type == NULL)
            return NULL;
        llvmTypes.push_back(type->LLVMType(ctx));
    }
    return llvm::StructType::get(*ctx, llvmTypes);
}


llvm::DIType
StructType::GetDIType(llvm::DIDescriptor scope) const {
    uint64_t currentSize = 0, align = 0;

    std::vector<llvm::Value *> elementLLVMTypes;
    // Walk through the elements of the struct; for each one figure out its
    // alignment and size, using that to figure out its offset w.r.t. the
    // start of the structure.
    for (unsigned int i = 0; i < elementTypes.size(); ++i) {
        llvm::DIType eltType = GetElementType(i)->GetDIType(scope);
        uint64_t eltAlign = eltType.getAlignInBits();
        uint64_t eltSize = eltType.getSizeInBits();

        // The alignment for the entire structure is the maximum of the
        // required alignments of its elements
        align = std::max(align, eltAlign);

        // Move the current size forward if needed so that the current
        // element starts at an offset that's the correct alignment.
        if (currentSize > 0 && (currentSize % eltAlign))
            currentSize += eltAlign - (currentSize % eltAlign);
        Assert((currentSize == 0) || (currentSize % eltAlign) == 0);

        llvm::DIFile diFile = elementPositions[i].GetDIFile();
        int line = elementPositions[i].first_line;
#ifdef LLVM_2_9
        llvm::DIType fieldType = 
            m->diBuilder->createMemberType(elementNames[i], diFile, line,
                                           eltSize, eltAlign, currentSize, 0,
                                           eltType);
#else
        llvm::DIType fieldType = 
            m->diBuilder->createMemberType(scope, elementNames[i], diFile, 
                                           line, eltSize, eltAlign, 
                                           currentSize, 0, eltType);
#endif // LLVM_2_9
        elementLLVMTypes.push_back(fieldType);

        currentSize += eltSize;
    }

    // Round up the struct's entire size so that it's a multiple of the
    // required alignment that we figured out along the way...
    if (currentSize > 0 && (currentSize % align))
        currentSize += align - (currentSize % align);

#ifdef LLVM_2_9
    llvm::DIArray elements = m->diBuilder->getOrCreateArray(&elementLLVMTypes[0], 
                                                            elementLLVMTypes.size());
#else
    llvm::DIArray elements = m->diBuilder->getOrCreateArray(elementLLVMTypes);
#endif
    llvm::DIFile diFile = pos.GetDIFile();
    return m->diBuilder->createStructType(scope, name, diFile, pos.first_line, currentSize, 
                                          align, 0, elements);
}


const Type *
StructType::GetElementType(int i) const {
    Assert(variability != Variability::Unbound);
    Assert(i < (int)elementTypes.size());
    const Type *ret = elementTypes[i];
    if (ret == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }

    // If the element has unbound variability, resolve its variability to
    // the struct type's variability
    ret = ret->ResolveUnboundVariability(variability);

    return isConst ? ret->GetAsConstType() : ret;
}


const Type *
StructType::GetElementType(const std::string &n) const {
    for (unsigned int i = 0; i < elementNames.size(); ++i)
        if (elementNames[i] == n)
            return GetElementType(i);
    return NULL;
}


int
StructType::GetElementNumber(const std::string &n) const {
    for (unsigned int i = 0; i < elementNames.size(); ++i)
        if (elementNames[i] == n)
            return i;
    return -1;
}


bool
StructType::checkIfCanBeSOA(const StructType *st) {
    bool ok = true;
    for (int i = 0; i < (int)st->elementTypes.size(); ++i) {
        const Type *eltType = st->elementTypes[i];
        const StructType *childStructType = 
            dynamic_cast<const StructType *>(eltType);

        if (childStructType != NULL)
            ok &= checkIfCanBeSOA(childStructType);
        else if (eltType->HasUnboundVariability() == false) {
            Error(st->elementPositions[i], "Unable to apply SOA conversion to "
                  "struct due to \"%s\" member \"%s\" with bound \"%s\" "
                  "variability.", eltType->GetString().c_str(),
                  st->elementNames[i].c_str(), 
                  eltType->IsUniformType() ? "uniform" : "varying");
            ok = false;
        }
        else if (dynamic_cast<const ReferenceType *>(eltType)) {
            Error(st->elementPositions[i], "Unable to apply SOA conversion to "
                  "struct due to member \"%s\" with reference type \"%s\".",
                  st->elementNames[i].c_str(), eltType->GetString().c_str());
            ok = false;
        }
    }
    return ok;
}


///////////////////////////////////////////////////////////////////////////
// ReferenceType

ReferenceType::ReferenceType(const Type *t) 
    : targetType(t) {
}


Variability
ReferenceType::GetVariability() const {
    if (targetType == NULL) {
        Assert(m->errorCount > 0);
        return Variability(Variability::Unbound);
    }
    return targetType->GetVariability(); 
}


bool
ReferenceType::IsBoolType() const {
    if (targetType == NULL) {
        Assert(m->errorCount > 0);
        return false;
    }
    return targetType->IsBoolType(); 
}


bool
ReferenceType::IsFloatType() const {
    if (targetType == NULL) {
        Assert(m->errorCount > 0);
        return false;
    }
    return targetType->IsFloatType(); 
}


bool
ReferenceType::IsIntType() const {
    if (targetType == NULL) {
        Assert(m->errorCount > 0);
        return false;
    }
    return targetType->IsIntType(); 
}


bool
ReferenceType::IsUnsignedType() const {
    if (targetType == NULL) {
        Assert(m->errorCount > 0);
        return false;
    }
    return targetType->IsUnsignedType(); 
}


bool
ReferenceType::IsConstType() const {
    if (targetType == NULL) {
        Assert(m->errorCount > 0);
        return false;
    }
    return targetType->IsConstType();
}


const Type *
ReferenceType::GetReferenceTarget() const {
    return targetType;
}


const Type *
ReferenceType::GetBaseType() const {
    if (targetType == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }
    return targetType->GetBaseType();
}


const ReferenceType *
ReferenceType::GetAsVaryingType() const {
    if (targetType == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }
    if (IsVaryingType()) 
        return this;
    return new ReferenceType(targetType->GetAsVaryingType());
}


const ReferenceType *
ReferenceType::GetAsUniformType() const {
    if (targetType == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }
    if (IsUniformType()) 
        return this;
    return new ReferenceType(targetType->GetAsUniformType());
}


const ReferenceType *
ReferenceType::GetAsUnboundVariabilityType() const {
    if (targetType == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }
    if (HasUnboundVariability()) 
        return this;
    return new ReferenceType(targetType->GetAsUnboundVariabilityType());
}


const Type *
ReferenceType::GetAsSOAType(int width) const {
    // FIXME: is this right?
    return new ArrayType(this, width);
}


const ReferenceType *
ReferenceType::ResolveUnboundVariability(Variability v) const {
    if (targetType == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }
    return new ReferenceType(targetType->ResolveUnboundVariability(v));
}
    

const ReferenceType *
ReferenceType::GetAsConstType() const {
    if (targetType == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }
    if (IsConstType())
        return this;
    return new ReferenceType(targetType->GetAsConstType());
}


const ReferenceType *
ReferenceType::GetAsNonConstType() const {
    if (targetType == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }
    if (!IsConstType())
        return this;
    return new ReferenceType(targetType->GetAsNonConstType());
}


std::string
ReferenceType::GetString() const {
    if (targetType == NULL) {
        Assert(m->errorCount > 0);
        return "";
    }

    std::string ret = targetType->GetString();

    ret += std::string(" &");
    return ret;
}


std::string
ReferenceType::Mangle() const {
    if (targetType == NULL) {
        Assert(m->errorCount > 0);
        return "";
    }
    std::string ret;
    ret += std::string("REF") + targetType->Mangle();
    return ret;
}


std::string
ReferenceType::GetCDeclaration(const std::string &name) const {
    if (targetType == NULL) {
        Assert(m->errorCount > 0);
        return "";
    }

    const ArrayType *at = dynamic_cast<const ArrayType *>(targetType);
    if (at != NULL) {
        if (at->GetElementCount() == 0) {
            // emit unsized arrays as pointers to the base type..
            std::string ret;
            ret += at->GetElementType()->GetAsNonConstType()->GetCDeclaration("") + 
                std::string(" *");
            if (lShouldPrintName(name))
                ret += name;
            return ret;
        }
        else
            // otherwise forget about the reference part if it's an
            // array since C already passes arrays by reference...
            return targetType->GetCDeclaration(name);
    }
    else {
        std::string ret;
        ret += targetType->GetCDeclaration("") + std::string(" *");
        if (lShouldPrintName(name))
            ret += name;
        return ret;
    }
}


LLVM_TYPE_CONST llvm::Type *
ReferenceType::LLVMType(llvm::LLVMContext *ctx) const {
    if (targetType == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }

    LLVM_TYPE_CONST llvm::Type *t = targetType->LLVMType(ctx);
    if (t == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }

    return llvm::PointerType::get(t, 0);
}


llvm::DIType
ReferenceType::GetDIType(llvm::DIDescriptor scope) const {
    if (targetType == NULL) {
        Assert(m->errorCount > 0);
        return llvm::DIType();
    }

    llvm::DIType diTargetType = targetType->GetDIType(scope);
    return m->diBuilder->createReferenceType(diTargetType);
}


///////////////////////////////////////////////////////////////////////////
// FunctionType

FunctionType::FunctionType(const Type *r, const std::vector<const Type *> &a, 
                           SourcePos p)
    : isTask(false), isExported(false), isExternC(false), returnType(r), 
      paramTypes(a), paramNames(std::vector<std::string>(a.size(), "")),
      paramDefaults(std::vector<ConstExpr *>(a.size(), NULL)),
      paramPositions(std::vector<SourcePos>(a.size(), p)) {
    Assert(returnType != NULL);
    isSafe = false;
    costOverride = -1;
}


FunctionType::FunctionType(const Type *r, const std::vector<const Type *> &a, 
                           const std::vector<std::string> &an, 
                           const std::vector<ConstExpr *> &ad,
                           const std::vector<SourcePos> &ap,
                           bool it, bool is, bool ec) 
    : isTask(it), isExported(is), isExternC(ec), returnType(r), paramTypes(a), 
      paramNames(an), paramDefaults(ad), paramPositions(ap) {
    Assert(paramTypes.size() == paramNames.size() && 
           paramNames.size() == paramDefaults.size() &&
           paramDefaults.size() == paramPositions.size());
    Assert(returnType != NULL);
    isSafe = false;
    costOverride = -1;
}


Variability
FunctionType::GetVariability() const {
    return Variability(Variability::Uniform);
}


bool
FunctionType::IsFloatType() const {
    return false;
}


bool
FunctionType::IsIntType() const {
    return false;
}


bool
FunctionType::IsBoolType() const {
    return false;
}


bool
FunctionType::IsUnsignedType() const {
    return false;
}


bool
FunctionType::IsConstType() const {
    return false;
}


const Type *
FunctionType::GetBaseType() const {
    FATAL("FunctionType::GetBaseType() shouldn't be called");
    return NULL;
}


const Type *
FunctionType::GetAsVaryingType() const {
    FATAL("FunctionType::GetAsVaryingType shouldn't be called");
    return NULL;
}


const Type *
FunctionType::GetAsUniformType() const {
    FATAL("FunctionType::GetAsUniformType shouldn't be called");
    return NULL;
}


const Type *
FunctionType::GetAsUnboundVariabilityType() const {
    FATAL("FunctionType::GetAsUnboundVariabilityType shouldn't be called");
    return NULL;
}


const Type *
FunctionType::GetAsSOAType(int width) const {
    FATAL("FunctionType::GetAsSOAType() shouldn't be called");
    return NULL;
}


const FunctionType *
FunctionType::ResolveUnboundVariability(Variability v) const {
    if (returnType == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }
    const Type *rt = returnType->ResolveUnboundVariability(v);

    std::vector<const Type *> pt;
    for (unsigned int i = 0; i < paramTypes.size(); ++i) {
        if (paramTypes[i] == NULL) {
            Assert(m->errorCount > 0);
            return NULL;
        }
        pt.push_back(paramTypes[i]->ResolveUnboundVariability(v));
    }

    FunctionType *ret = new FunctionType(rt, pt, paramNames, paramDefaults,
                                         paramPositions, isTask, isExported,
                                         isExternC);
    ret->isSafe = isSafe;
    ret->costOverride = costOverride;

    return ret;
}


const Type *
FunctionType::GetAsConstType() const {
    FATAL("FunctionType::GetAsConstType shouldn't be called");
    return NULL;
}


const Type *
FunctionType::GetAsNonConstType() const {
    FATAL("FunctionType::GetAsNonConstType shouldn't be called");
    return NULL;
}


std::string
FunctionType::GetString() const {
    std::string ret;
    if (isTask) ret += "task ";
    if (isSafe) ret += "/*safe*/ ";
    if (costOverride > 0) {
        char buf[32];
        sprintf(buf, "/*cost=%d*/ ", costOverride);
        ret += buf;
    }
    if (returnType != NULL)
        ret += returnType->GetString();
    else
        ret += "/* ERROR */";
    ret += "(";
    for (unsigned int i = 0; i < paramTypes.size(); ++i) {
        if (paramTypes[i] == NULL)
            ret += "/* ERROR */";
        else
            ret += paramTypes[i]->GetString();

        if (i != paramTypes.size() - 1)
            ret += ", ";
    }
    ret += ")";
    return ret;
}


std::string
FunctionType::Mangle() const {
    std::string ret = "___";
    for (unsigned int i = 0; i < paramTypes.size(); ++i)
        if (paramTypes[i] == NULL)
            Assert(m->errorCount > 0);
        else
            ret += paramTypes[i]->Mangle();

    return ret;
}


std::string
FunctionType::GetCDeclaration(const std::string &fname) const {
    std::string ret;
    ret += returnType->GetCDeclaration("");
    ret += " ";
    ret += fname;
    ret += "(";
    for (unsigned int i = 0; i < paramTypes.size(); ++i) {
        const Type *type = paramTypes[i];

        // Convert pointers to arrays to unsized arrays, which are more clear
        // to print out for multidimensional arrays (i.e. "float foo[][4] "
        // versus "float (foo *)[4]").
        const PointerType *pt = dynamic_cast<const PointerType *>(type);
        if (pt != NULL && 
            dynamic_cast<const ArrayType *>(pt->GetBaseType()) != NULL) {
            type = new ArrayType(pt->GetBaseType(), 0);
        }

        if (paramNames[i] != "")
            ret += type->GetCDeclaration(paramNames[i]);
        else
            ret += type->GetString();
        if (i != paramTypes.size() - 1)
            ret += ", ";
    }
    ret += ")";
    return ret;
}


LLVM_TYPE_CONST llvm::Type *
FunctionType::LLVMType(llvm::LLVMContext *ctx) const {
    FATAL("FunctionType::LLVMType() shouldn't be called");
    return NULL;
}


llvm::DIType
FunctionType::GetDIType(llvm::DIDescriptor scope) const {
    // @todo need to implement FunctionType::GetDIType()
    FATAL("need to implement FunctionType::GetDIType()");
    return llvm::DIType();
}


LLVM_TYPE_CONST llvm::FunctionType *
FunctionType::LLVMFunctionType(llvm::LLVMContext *ctx, bool includeMask) const {
    if (isTask == true) 
        Assert(includeMask == true);

    // Get the LLVM Type *s for the function arguments
    std::vector<LLVM_TYPE_CONST llvm::Type *> llvmArgTypes;
    for (unsigned int i = 0; i < paramTypes.size(); ++i) {
        if (paramTypes[i] == NULL) {
            Assert(m->errorCount > 0);
            return NULL;
        }
        Assert(Type::Equal(paramTypes[i], AtomicType::Void) == false);

        LLVM_TYPE_CONST llvm::Type *t = paramTypes[i]->LLVMType(ctx);
        if (t == NULL) {
            Assert(m->errorCount > 0);
            return NULL;
        }
        llvmArgTypes.push_back(t);
    }

    // And add the function mask, if asked for
    if (includeMask)
        llvmArgTypes.push_back(LLVMTypes::MaskType);

    std::vector<LLVM_TYPE_CONST llvm::Type *> callTypes;
    if (isTask) {
        // Tasks take three arguments: a pointer to a struct that holds the
        // actual task arguments, the thread index, and the total number of
        // threads the tasks system has running.  (Task arguments are
        // marshalled in a struct so that it's easy to allocate space to
        // hold them until the task actually runs.)
        llvm::Type *st = llvm::StructType::get(*ctx, llvmArgTypes);
        callTypes.push_back(llvm::PointerType::getUnqual(st));
        callTypes.push_back(LLVMTypes::Int32Type); // threadIndex
        callTypes.push_back(LLVMTypes::Int32Type); // threadCount
        callTypes.push_back(LLVMTypes::Int32Type); // taskIndex
        callTypes.push_back(LLVMTypes::Int32Type); // taskCount
    }
    else
        // Otherwise we already have the types of the arguments 
        callTypes = llvmArgTypes;

    if (returnType == NULL) {
        Assert(m->errorCount > 0);
        return NULL;
    }

    return llvm::FunctionType::get(returnType->LLVMType(g->ctx), callTypes, false);
}


const Type *
FunctionType::GetParameterType(int i) const { 
    Assert(i < (int)paramTypes.size());
    return paramTypes[i];
}


ConstExpr *
FunctionType::GetParameterDefault(int i) const { 
    Assert(i < (int)paramDefaults.size());
    return paramDefaults[i]; 
}


const SourcePos &
FunctionType::GetParameterSourcePos(int i) const { 
    Assert(i < (int)paramPositions.size());
    return paramPositions[i];
}


const std::string &
FunctionType::GetParameterName(int i) const { 
    Assert(i < (int)paramNames.size());
    return paramNames[i]; 
}


///////////////////////////////////////////////////////////////////////////
// Type

const Type *
Type::GetReferenceTarget() const {
    // only ReferenceType needs to override this method
    return this;
}


const Type *
Type::GetAsUnsignedType() const {
    // For many types, this doesn't make any sesne
    return NULL;
}


/** Given an atomic or vector type, return a vector type of the given
    vecSize.  Issue an error if given a vector type that isn't already that
    size.
 */
static const Type *
lVectorConvert(const Type *type, SourcePos pos, const char *reason, int vecSize) {
    const VectorType *vt = dynamic_cast<const VectorType *>(type);
    if (vt) {
        if (vt->GetElementCount() != vecSize) {
            Error(pos, "Implicit conversion between from vector type "
                  "\"%s\" to vector type of length %d for %s is not possible.", 
                  type->GetString().c_str(), vecSize, reason);
            return NULL;
        }
        return vt;
    }
    else {
        const AtomicType *at = dynamic_cast<const AtomicType *>(type);
        if (!at) {
            Error(pos, "Non-atomic type \"%s\" can't be converted to vector type "
                  "for %s.", type->GetString().c_str(), reason);
            return NULL;
        }
        return new VectorType(at, vecSize);
    }
}


const Type *
Type::MoreGeneralType(const Type *t0, const Type *t1, SourcePos pos, const char *reason, 
                      bool forceVarying, int vecSize) {
    Assert(reason != NULL);

    // First, if we need to go varying, promote both of the types to be
    // varying.
    if (t0->IsVaryingType() || t1->IsVaryingType() || forceVarying) {
        t0 = t0->GetAsVaryingType();
        t1 = t1->GetAsVaryingType();
    }

    // And similarly, promote them both to vectors if the caller requested
    // a particular vector size
    if (vecSize > 0) {
        t0 = lVectorConvert(t0, pos, reason, vecSize);
        t1 = lVectorConvert(t1, pos, reason, vecSize);
        if (!t0 || !t1)
            return NULL;
    }

    // Are they both the same type?  If so, we're done, QED.
    if (Type::Equal(t0, t1)) 
        return t0;
    
    // If they're function types, it's hopeless if they didn't match in the
    // Type::Equal() call above.  Fail here so that we don't get into
    // trouble calling GetAsConstType()...
    if (dynamic_cast<const FunctionType *>(t0) ||
        dynamic_cast<const FunctionType *>(t1)) {
        Error(pos, "Incompatible function types \"%s\" and \"%s\" in %s.",
              t0->GetString().c_str(), t1->GetString().c_str(), reason);
        return NULL;
    }

    // Not the same types, but only a const/non-const difference?  Return
    // the non-const type as the more general one.
    if (Type::EqualIgnoringConst(t0, t1))
        return t0->GetAsNonConstType();

    const PointerType *pt0 = dynamic_cast<const PointerType *>(t0);
    const PointerType *pt1 = dynamic_cast<const PointerType *>(t1);
    if (pt0 != NULL && pt1 != NULL) {
        if (PointerType::IsVoidPointer(pt0))
            return pt1;
        else if (PointerType::IsVoidPointer(pt1))
            return pt0;
        else {
            Error(pos, "Conversion between incompatible pointer types \"%s\" "
                  "and \"%s\" isn't possible.", t0->GetString().c_str(),
                  t1->GetString().c_str());
            return NULL;
        }
    }

    const VectorType *vt0 = dynamic_cast<const VectorType *>(t0);
    const VectorType *vt1 = dynamic_cast<const VectorType *>(t1);
    if (vt0 && vt1) {
        // both are vectors; convert their base types and make a new vector
        // type, as long as their lengths match
        if (vt0->GetElementCount() != vt1->GetElementCount()) {
            Error(pos, "Implicit conversion between differently sized vector types "
                  "(%s, %s) for %s is not possible.", t0->GetString().c_str(),
                  t1->GetString().c_str(), reason);
            return NULL;
        }
        const Type *t = MoreGeneralType(vt0->GetElementType(), vt1->GetElementType(),
                                        pos, reason, forceVarying);
        if (!t) 
            return NULL;

        // The 'more general' version of the two vector element types must
        // be an AtomicType (that's all that vectors can hold...)
        const AtomicType *at = dynamic_cast<const AtomicType *>(t);
        Assert(at != NULL);

        return new VectorType(at, vt0->GetElementCount());
    }
    else if (vt0) {
        // If one type is a vector type but the other isn't, see if we can
        // promote the other one to a vector type.  This will fail and
        // return NULL if t1 is e.g. an array type and it's illegal to have
        // a vector of it..
        const Type *t = MoreGeneralType(vt0->GetElementType(), t1, pos, 
                                        reason, forceVarying);
        if (!t) 
            return NULL;

        const AtomicType *at = dynamic_cast<const AtomicType *>(t);
        Assert(at != NULL);
        return new VectorType(at, vt0->GetElementCount());
    }
    else if (vt1) {
        // As in the above case, see if we can promote t0 to make a vector
        // that matches vt1.
        const Type *t = MoreGeneralType(t0, vt1->GetElementType(), pos, 
                                        reason, forceVarying);
        if (!t) 
            return NULL;

        const AtomicType *at = dynamic_cast<const AtomicType *>(t);
        Assert(at != NULL);
        return new VectorType(at, vt1->GetElementCount());
    }

    // TODO: what do we need to do about references here, if anything??

    const AtomicType *at0 = dynamic_cast<const AtomicType *>(t0->GetReferenceTarget());
    const AtomicType *at1 = dynamic_cast<const AtomicType *>(t1->GetReferenceTarget());

    const EnumType *et0 = dynamic_cast<const EnumType *>(t0->GetReferenceTarget());
    const EnumType *et1 = dynamic_cast<const EnumType *>(t1->GetReferenceTarget());
    if (et0 != NULL && et1 != NULL) {
        // Two different enum types -> make them uint32s...
        Assert(et0->IsVaryingType() == et1->IsVaryingType());
        return et0->IsVaryingType() ? AtomicType::VaryingUInt32 :
                AtomicType::UniformUInt32;
    }
    else if (et0 != NULL) {
        if (at1 != NULL)
            // Enum type and atomic type -> convert the enum to the atomic type
            // TODO: should we return uint32 here, unless the atomic type is
            // a 64-bit atomic type, in which case we return that?
            return at1;
        else {
            Error(pos, "Implicit conversion from enum type \"%s\" to "
                  "non-atomic type \"%s\" for %s not possible.",
                  t0->GetString().c_str(), t1->GetString().c_str(), reason);
            return NULL;
        }
    }
    else if (et1 != NULL) {
        if (at0 != NULL)
            // Enum type and atomic type; see TODO above here as well...
            return at0;
        else {
            Error(pos, "Implicit conversion from enum type \"%s\" to "
                  "non-atomic type \"%s\" for %s not possible.",
                  t1->GetString().c_str(), t0->GetString().c_str(), reason);
            return NULL;
        }
    }

    // Now all we can do is promote atomic types...
    if (at0 == NULL || at1 == NULL) {
        Assert(reason != NULL);
        Error(pos, "Implicit conversion from type \"%s\" to \"%s\" for %s not possible.",
              t0->GetString().c_str(), t1->GetString().c_str(), reason);
        return NULL;
    }

    // Finally, to determine which of the two atomic types is more general,
    // use the ordering of entries in the AtomicType::BasicType enumerator.
    return (int(at0->basicType) >= int(at1->basicType)) ? at0 : at1;
}


bool
Type::IsBasicType(const Type *type) {
    return (dynamic_cast<const AtomicType *>(type) != NULL ||
            dynamic_cast<const EnumType *>(type) != NULL ||
            dynamic_cast<const PointerType *>(type) != NULL);
}


static bool
lCheckTypeEquality(const Type *a, const Type *b, bool ignoreConst) {
    if (a == NULL || b == NULL)
        return false;

    if (ignoreConst == true) {
        if (dynamic_cast<const FunctionType *>(a) == NULL)
            a = a->GetAsNonConstType();
        if (dynamic_cast<const FunctionType *>(b) == NULL)
            b = b->GetAsNonConstType();
    }
    else if (a->IsConstType() != b->IsConstType())
        return false;

    const AtomicType *ata = dynamic_cast<const AtomicType *>(a);
    const AtomicType *atb = dynamic_cast<const AtomicType *>(b);
    if (ata != NULL && atb != NULL) {
        return ((ata->basicType == atb->basicType) && 
                (ata->GetVariability() == atb->GetVariability()));
    }

    // For all of the other types, we need to see if we have the same two
    // general types.  If so, then we dig into the details of the type and
    // see if all of the relevant bits are equal...
    const EnumType *eta = dynamic_cast<const EnumType *>(a);
    const EnumType *etb = dynamic_cast<const EnumType *>(b);
    if (eta != NULL && etb != NULL)
        // Kind of goofy, but this sufficies to check
        return (eta->pos == etb->pos &&
                eta->GetVariability() == etb->GetVariability());

    const ArrayType *arta = dynamic_cast<const ArrayType *>(a);
    const ArrayType *artb = dynamic_cast<const ArrayType *>(b);
    if (arta != NULL && artb != NULL)
        return (arta->GetElementCount() == artb->GetElementCount() && 
                lCheckTypeEquality(arta->GetElementType(), artb->GetElementType(), 
                                   ignoreConst));

    const VectorType *vta = dynamic_cast<const VectorType *>(a);
    const VectorType *vtb = dynamic_cast<const VectorType *>(b);
    if (vta != NULL && vtb != NULL)
        return (vta->GetElementCount() == vtb->GetElementCount() && 
                lCheckTypeEquality(vta->GetElementType(), vtb->GetElementType(),
                                   ignoreConst));

    const StructType *sta = dynamic_cast<const StructType *>(a);
    const StructType *stb = dynamic_cast<const StructType *>(b);
    if (sta != NULL && stb != NULL) {
        if (sta->GetElementCount() != stb->GetElementCount())
            return false;
        if (sta->GetStructName() != stb->GetStructName())
            return false;
        if (sta->GetVariability() != stb->GetVariability())
            return false;
        for (int i = 0; i < sta->GetElementCount(); ++i)
            // FIXME: is this redundant now?
            if (!lCheckTypeEquality(sta->GetElementType(i), stb->GetElementType(i),
                                    ignoreConst))
                return false;

        return true;
    }

    const PointerType *pta = dynamic_cast<const PointerType *>(a);
    const PointerType *ptb = dynamic_cast<const PointerType *>(b);
    if (pta != NULL && ptb != NULL)
        return (pta->IsUniformType() == ptb->IsUniformType() &&
                pta->IsSlice() == ptb->IsSlice() &&
                pta->IsFrozenSlice() == ptb->IsFrozenSlice() &&
                lCheckTypeEquality(pta->GetBaseType(), ptb->GetBaseType(), 
                                   ignoreConst));

    const ReferenceType *rta = dynamic_cast<const ReferenceType *>(a);
    const ReferenceType *rtb = dynamic_cast<const ReferenceType *>(b);
    if (rta != NULL && rtb != NULL)
        return (lCheckTypeEquality(rta->GetReferenceTarget(),
                                   rtb->GetReferenceTarget(), ignoreConst));

    const FunctionType *fta = dynamic_cast<const FunctionType *>(a);
    const FunctionType *ftb = dynamic_cast<const FunctionType *>(b);
    if (fta != NULL && ftb != NULL) {
        // Both the return types and all of the argument types must match
        // for function types to match
        if (!lCheckTypeEquality(fta->GetReturnType(), ftb->GetReturnType(), 
                                ignoreConst))
            return false;

        if (fta->isTask != ftb->isTask ||
            fta->isExported != ftb->isExported ||
            fta->isExternC != ftb->isExternC)
            return false;

        if (fta->GetNumParameters() != ftb->GetNumParameters())
            return false;

        for (int i = 0; i < fta->GetNumParameters(); ++i)
            if (!lCheckTypeEquality(fta->GetParameterType(i),
                       ftb->GetParameterType(i), ignoreConst))
                return false;

        return true;
    }

    return false;
}


bool
Type::Equal(const Type *a, const Type *b) {
    return lCheckTypeEquality(a, b, false);
}


bool
Type::EqualIgnoringConst(const Type *a, const Type *b) {
    return lCheckTypeEquality(a, b, true);
}
