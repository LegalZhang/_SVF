

//===- BreakConstantGEPs.cpp - Change constant GEPs into GEP instructions - --//
//
//                          The SAFECode Compiler
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass changes all GEP constant expressions into GEP instructions.  This
// permits the rest of SAFECode to put run-time checks on them if necessary.
//
//===----------------------------------------------------------------------===//


#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/InstIterator.h"

#include "SVF-LLVM/BasicTypes.h"
#include "SVF-LLVM/BreakConstantExpr.h"
#include "Util/GeneralType.h"
#include "Util/SVFUtil.h"

#include <iostream>
#include <map>
#include <utility>

using namespace SVF;

// Identifier variable for the pass
char BreakConstantGEPs::ID = 0;
char MergeFunctionRets::ID = 0;

#define DEBUG_TYPE "break-constgeps"

// Statistics
STATISTIC (GEPChanges,   "Number of Converted GEP Constant Expressions");
STATISTIC (TotalChanges, "Number of Converted Constant Expressions");

//
// Function: hasConstantGEP()
//
// Description:
//  This function determines whether the given value is a constant expression
//  that has a constant GEP expression embedded within it.
//
// Inputs:
//  V - The value to check.
//
// Return value:
//  nullptr  - This value is not a constant expression with a constant expression
//          GEP within it.
//  ~nullptr - A pointer to the value casted into a ConstantExpr is returned.
//
static ConstantExpr *
hasConstantGEP (Value*  V)
{
    if (ConstantExpr * CE = SVFUtil::dyn_cast<ConstantExpr>(V))
    {
        if (CE->getOpcode() == Instruction::GetElementPtr)
        {
            return CE;
        }
        else
        {
            for (u32_t index = 0; index < CE->getNumOperands(); ++index)
            {
                if (hasConstantGEP (CE->getOperand(index)))
                    return CE;
            }
        }
    }

    return nullptr;
}

// Description:
//  This function determines whether the given value is a constant expression
//  that has a constant binary or unary operator expression embedded within it.
static ConstantExpr *
hasConstantBinaryOrUnaryOp (Value*  V)
{
    if (ConstantExpr * CE = SVFUtil::dyn_cast<ConstantExpr>(V))
    {
        if (Instruction::isBinaryOp(CE->getOpcode()) || Instruction::isUnaryOp(CE->getOpcode()))
        {
            return CE;
        }
        else
        {
            for (u32_t index = 0; index < CE->getNumOperands(); ++index)
            {
                if (hasConstantBinaryOrUnaryOp (CE->getOperand(index)))
                    return CE;
            }
        }
    }

    return nullptr;
}

// Description:
// Return true if this is a constant Gep or binaryOp or UnaryOp expression
static ConstantExpr *
hasConstantExpr (Value*  V)
{
    if (ConstantExpr * gep = hasConstantGEP(V))
    {
        return gep;
    }
    else if (ConstantExpr * buop = hasConstantBinaryOrUnaryOp(V))
    {
        return buop;
    }
    else
    {
        return nullptr;
    }
}


//
// Function: convertExpression()
//
// Description:
//  Convert a constant expression into an instruction.  This routine does *not*
//  perform any recursion, so the resulting instruction may have constant
//  expression operands.
//
static Instruction*
convertExpression (ConstantExpr * CE, Instruction*  InsertPt)
{
    //
    // Convert this constant expression into a regular instruction.
    //
    if (CE->getOpcode() == Instruction::GetElementPtr)
        ++GEPChanges;
    ++TotalChanges;
    Instruction* Result = CE->getAsInstruction();
    Result->insertBefore(InsertPt);
    return Result;
}

//
// Method: runOnFunction()
//
// Description:
//  Entry point for this LLVM pass.
//
// Return value:
//  true  - The function was modified.
//  false - The function was not modified.
//
bool
BreakConstantGEPs::runOnModule (Module & module)
{
    bool modified = false;
    for (Module::iterator F = module.begin(), E = module.end(); F != E; ++F)
    {
        // Worklist of values to check for constant GEP expressions
        std::vector<Instruction* > Worklist;

        //
        // Initialize the worklist by finding all instructions that have one or more
        // operands containing a constant GEP expression.
        //
        for (Function::iterator BB = (*F).begin(); BB != (*F).end(); ++BB)
        {
            for (BasicBlock::iterator i = BB->begin(); i != BB->end(); ++i)
            {
                //
                // Scan through the operands of this instruction.  If it is a constant
                // expression GEP, insert an instruction GEP before the instruction.
                //
                Instruction*  I = &(*i);
                for (u32_t index = 0; index < I->getNumOperands(); ++index)
                {
                    if (hasConstantExpr(I->getOperand(index)))
                    {
                        Worklist.push_back (I);
                    }
                }
            }
        }

        //
        // Determine whether we will modify anything.
        //
        if (Worklist.size()) modified = true;

        //
        // While the worklist is not empty, take an item from it, convert the
        // operands into instructions if necessary, and determine if the newly
        // added instructions need to be processed as well.
        //
        while (Worklist.size())
        {
            Instruction*  I = Worklist.back();
            Worklist.pop_back();

            //
            // Scan through the operands of this instruction and convert each into an
            // instruction.  Note that this works a little differently for phi
            // instructions because the new instruction must be added to the
            // appropriate predecessor block.
            //
            if (PHINode * PHI = SVFUtil::dyn_cast<PHINode>(I))
            {
                for (u32_t index = 0; index < PHI->getNumIncomingValues(); ++index)
                {
                    //
                    // For PHI Nodes, if an operand is a constant expression with a GEP, we
                    // want to insert the new instructions in the predecessor basic block.
                    //
                    // Note: It seems that it's possible for a phi to have the same
                    // incoming basic block listed multiple times; this seems okay as long
                    // the same value is listed for the incoming block.
                    //
                    Instruction*  InsertPt = PHI->getIncomingBlock(index)->getTerminator();
                    if (ConstantExpr * CE = hasConstantExpr(PHI->getIncomingValue(index)))
                    {
                        Instruction*  NewInst = convertExpression (CE, InsertPt);
                        for (u32_t i2 = index; i2 < PHI->getNumIncomingValues(); ++i2)
                        {
                            if ((PHI->getIncomingBlock (i2)) == PHI->getIncomingBlock (index))
                                PHI->setIncomingValue (i2, NewInst);
                        }
                        Worklist.push_back (NewInst);
                    }
                }
            }
            else
            {
                for (u32_t index = 0; index < I->getNumOperands(); ++index)
                {
                    //
                    // For other instructions, we want to insert instructions replacing
                    // constant expressions immediately before the instruction using the
                    // constant expression.
                    //
                    if (ConstantExpr * CE = hasConstantExpr(I->getOperand(index)))
                    {
                        Instruction*  NewInst = convertExpression (CE, I);
                        I->replaceUsesOfWith (CE, NewInst);
                        Worklist.push_back (NewInst);
                    }
                }
            }
        }

    }
    return modified;
}




