/*
 * Copyright (c) 2001-2005 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Steve Reinhardt
 *          Lisa Hsu
 *          Nathan Binkert
 *          Steve Raasch
 */

#include <iomanip>

#include "arch/isa_traits.hh"
#include "arch/utility.hh"
#include "base/loader/symtab.hh"
#include "config/the_isa.hh"
#include "cpu/base.hh"
#include "cpu/exetrace.hh"
#include "cpu/static_inst.hh"
#include "cpu/thread_context.hh"
#include "debug/ExecAll.hh"
#include "enums/OpClass.hh"

using namespace std;
using namespace TheISA;

namespace Trace {

void
ExeTracerRecord::dumpTicks(ostream &outs)
{
    ccprintf(outs, "%7d: ", when);
}
enum Aarch64 {X0, X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15, X16, X17, X18, X19, X20, X21, X22, X23, X24, X25, X26, X27, X28, X29, W30, SP=43 , xzr=31 };
 
bool ExeTracerRecord::isMasterReg(int reg)
{
if (reg == Aarch64::X0 || reg == Aarch64::X1 || reg == Aarch64::X2 || reg == Aarch64::X3 || reg == Aarch64::X19 || reg == Aarch64::X20 || reg == Aarch64::X23 || reg == Aarch64::X24 || reg == Aarch64::X29 || reg == Aarch64::W30 || reg == Aarch64::SP) return true;
return false;
}
bool mustPrint=false;
bool printOriginal=false;
int instCount=0;
void
Trace::ExeTracerRecord::traceInst(const StaticInstPtr &inst, bool ran)
{

bool srcIsMaster=true;
bool desIsMaster=true;
ostream &outs = Trace::output();
    std::string sym_str,funcName;
    Addr sym_addr;
    Addr cur_pc = pc.instAddr();
debugSymbolTable->findNearestSymbol(cur_pc, funcName, sym_addr);


//printing the first instruction from the recovery BB
if ( (funcName == "main" || ((funcName[0] == 'F' && funcName[1] == 'U' && funcName[2] == 'N' && funcName[3] == 'C')))) {
//printout "add   x25, sp, xzr" 
if ( (inst->getName() == "add") && (inst->numDestRegs() == 1) && (inst->destRegIdx(0) == 25) && (inst->numSrcRegs() == 2) && (inst->srcRegIdx(0) == 31) && (inst->srcRegIdx(1) == 43))
 mustPrint=true;
//printout and x25, x25, x3  
if ( (inst->getName() == "and") && (inst->numDestRegs() == 1) && (inst->destRegIdx(0) == 25) && (inst->numSrcRegs() == 2) && (inst->srcRegIdx(0) == 3) && (inst->srcRegIdx(1) == 25))
 mustPrint=true;
//printout "csel   x27, x25, x27, eq" 
if ( (inst->getName() == "csel") && (inst->numDestRegs() == 1) && (inst->destRegIdx(0) == 27) && (inst->numSrcRegs() == 5) && (inst->srcRegIdx(0) == 1536) && (inst->srcRegIdx(1) == 1537) && (inst->srcRegIdx(2) == 1538) && (inst->srcRegIdx(3) == 25) && (inst->srcRegIdx(4) == 27)  )mustPrint=true;

}



//std::cout << "inst->debugEnd = " << inst->debugEnd << " && when  " << when << "\n";
if ( ((funcName == "main" || ((funcName[0] == 'F' && funcName[1] == 'U' && funcName[2] == 'N' && funcName[3] == 'C'))) && (instCount < 1000 ) ) || mustPrint )
{
instCount++;
unsigned int num_src_regs = inst->numSrcRegs();
unsigned int num_dest_regs = inst->numDestRegs();
			unsigned int src_reg = 0, des_reg = 0;
			while (des_reg < num_dest_regs  && (inst->opClass() == IntAluOp) ) {
				if (!isMasterReg(inst->destRegIdx(des_reg)) && inst->destRegIdx(des_reg) < 32) {desIsMaster=false; break;}
				des_reg++;
			}
			while (src_reg < num_src_regs  && (inst->opClass() == IntAluOp) ) {
				//outs << " src_reg( " << src_reg << ") = " << inst->srcRegIdx(src_reg);
				if (!isMasterReg(inst->srcRegIdx(src_reg)) && inst->srcRegIdx(src_reg) < 32) {srcIsMaster=false; break;}
				src_reg++;
			}
			//outs << "\n";


if ( (desIsMaster && srcIsMaster && printOriginal) || mustPrint)
{
mustPrint=false;
    

    if (!Debug::ExecUser || !Debug::ExecKernel) {
        bool in_user_mode = TheISA::inUserMode(thread);
        if (in_user_mode && !Debug::ExecUser) return;
        if (!in_user_mode && !Debug::ExecKernel) return;
    }

    if (Debug::ExecTicks)
        dumpTicks(outs);

    outs << thread->getCpuPtr()->name() << " ";

    if (Debug::ExecAsid)
        outs << "A" << dec << TheISA::getExecutingAsid(thread) << " ";

    if (Debug::ExecThread)
        outs << "T" << thread->threadId() << " : ";


    if (debugSymbolTable && Debug::ExecSymbol &&
            (!FullSystem || !inUserMode(thread)) &&
            debugSymbolTable->findNearestSymbol(cur_pc, sym_str, sym_addr)) {
        if (cur_pc != sym_addr)
            sym_str += csprintf("+%d",cur_pc - sym_addr);
        outs << "@" << sym_str;
    } else {
        outs << "0x" << hex << cur_pc;
    }

    if (inst->isMicroop()) {
        outs << "." << setw(2) << dec << pc.microPC();
    } else {
        outs << "   ";
    }

    outs << " : ";

    //
    //  Print decoded instruction
    //

    outs << setw(26) << left;
    outs << inst->disassemble(cur_pc, debugSymbolTable);

    if (ran) {
        outs << " : ";

        if (Debug::ExecOpClass) {
            outs << Enums::OpClassStrings[inst->opClass()] << " : ";
        }

        if (Debug::ExecResult && !predicate) {
            outs << "Predicated False";
        }

        if (Debug::ExecResult && data_status != DataInvalid) {
            ccprintf(outs, " D=%#018x", data.as_int);
        }

        if (Debug::ExecEffAddr && getMemValid())
            outs << " A=0x" << hex << addr;

        if (Debug::ExecFetchSeq && fetch_seq_valid)
            outs << "  FetchSeq=" << dec << fetch_seq;

        if (Debug::ExecCPSeq && cp_seq_valid)
            outs << "  CPSeq=" << dec << cp_seq;

        if (Debug::ExecFlags) {
            outs << "  flags=(";
            inst->printFlags(outs, "|");
            outs << ")";
        }
    }

    //
    //  End of line...
    //
    outs << endl;
}
else
{
   mustPrint=false; 

    if (!Debug::ExecUser || !Debug::ExecKernel) {
        bool in_user_mode = TheISA::inUserMode(thread);
        if (in_user_mode && !Debug::ExecUser) return;
        if (!in_user_mode && !Debug::ExecKernel) return;
    }

    if (Debug::ExecTicks)
        dumpTicks(outs);

    outs << thread->getCpuPtr()->name() << " ";

    if (Debug::ExecAsid)
        outs << "A" << dec << TheISA::getExecutingAsid(thread) << " ";

    if (Debug::ExecThread)
        outs << "T" << thread->threadId() << " : ";


    if (debugSymbolTable && Debug::ExecSymbol &&
            (!FullSystem || !inUserMode(thread)) &&
            debugSymbolTable->findNearestSymbol(cur_pc, sym_str, sym_addr)) {
        if (cur_pc != sym_addr)
            sym_str += csprintf("+%d",cur_pc - sym_addr);
        outs << "@" << sym_str;
    } else {
        outs << "0x" << hex << cur_pc;
    }

    if (inst->isMicroop()) {
        outs << "." << setw(2) << dec << pc.microPC();
    } else {
        outs << "   ";
    }

    outs << " : ";

    //
    //  Print decoded instruction
    //

    outs << setw(26) << left;
    outs << inst->disassemble(cur_pc, debugSymbolTable);

    if (ran) {
        outs << " : ";

        if (Debug::ExecOpClass) {
            outs << Enums::OpClassStrings[inst->opClass()] << " : ";
        }

        if (Debug::ExecResult && !predicate) {
            outs << "Predicated False";
        }

        if (Debug::ExecResult && data_status != DataInvalid) {
            ccprintf(outs, " D=%#018x", data.as_int);
        }

        if (Debug::ExecEffAddr && getMemValid())
            outs << " A=0x" << hex << addr;

        if (Debug::ExecFetchSeq && fetch_seq_valid)
            outs << "  FetchSeq=" << dec << fetch_seq;

        if (Debug::ExecCPSeq && cp_seq_valid)
            outs << "  CPSeq=" << dec << cp_seq;

        if (Debug::ExecFlags) {
            outs << "  flags=(";
            inst->printFlags(outs, "|");
            outs << ")";
        }
    }

    //
    //  End of line...
    //
    outs << endl;

}
}
}
void
Trace::ExeTracerRecord::dump()
{
    /*
     * The behavior this check tries to achieve is that if ExecMacro is on,
     * the macroop will be printed. If it's on and microops are also on, it's
     * printed before the microops start printing to give context. If the
     * microops aren't printed, then it's printed only when the final microop
     * finishes. Macroops then behave like regular instructions and don't
     * complete/print when they fault.
     */
    if (Debug::ExecMacro && staticInst->isMicroop() &&
        ((Debug::ExecMicro &&
            macroStaticInst && staticInst->isFirstMicroop()) ||
            (!Debug::ExecMicro &&
             macroStaticInst && staticInst->isLastMicroop()))) {
        traceInst(macroStaticInst, false);
    }
    if (Debug::ExecMicro || !staticInst->isMicroop()) {
        traceInst(staticInst, true);
    }
}

} // namespace Trace

////////////////////////////////////////////////////////////////////////
//
//  ExeTracer Simulation Object
//
Trace::ExeTracer *
ExeTracerParams::create()
{
    return new Trace::ExeTracer(this);
}
