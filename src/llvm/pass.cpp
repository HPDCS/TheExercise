#include "llvm/Pass.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/PassRegistry.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Function.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"

#include <iostream>
#include <vector>

using namespace llvm;

namespace {
	struct PocPass : public ModulePass {
		static char ID;

		PocPass() : ModulePass(ID) {}

		bool runOnModule(Module &M) {

			for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {

				//loop over each function within the file
				for (Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB) {
					//loop over each instruction inside function
					for (BasicBlock::iterator BI = BB->begin(), BE = BB->end(); BI != BE; ++BI) {
						MDNode *metadata;

						BI->print(errs());
						printf("\n");

						metadata = BI->getMetadata(1);
						printf("%d\n", metadata->get(1, 2));

						// SmallVector<std::pair<unsigned, MDNode *>, 4> MDs;
						// metadata = BI->getAllMetadata(&MDs);

						// printf("Instruction opcode: %d, operands: %d\n",
						// 	BI->getOpcode(),
						// 	BI->getNumOperands()
						// );

						// for(MDNode node : MDs) {
						// 	printf("%s\n", node->getOperand());
						// 	node->getOperand()->get()->dump();
						// }
						

						if(!BI->mayWriteToMemory()) {
							printf("Instruction does not (probably) write on memory\n");
							continue;
						}

						if (Instruction *inst = dyn_cast<StoreInst>(&(*BI))) {
							 inst;
							printf("Instruction: %s, opcode: %d\n", BI->getOpcodeName(), BI->getOpcode());

						} else if (isa<MemSetInst>(&(*BI))) {
							printf("Not implemented!\n");

						} else if (isa<MemCpyInst>(&(*BI))) {
							printf("Not implemented!\n");

						} else {
							if (isa<CallInst>(&(*BI))) continue;
							errs() << "[UNRECOGNIZED!1!]";
						}
					}
				}
			}
			return true;
		}


		void handleInstruction(Instruction *theInstruction, Module *M) {
			IRBuilder<> builder(M->getContext());
		}
	};
}


// Pass info
char PocPass::ID = 0; // LLVM ignores the actual value: it referes to the pointer.

// Pass loading stuff
// To use, run: clang -Xclang -load -Xclang <your-pass>.so <other-args> ...

// This function is of type PassManagerBuilder::ExtensionFn
static void loadPass(const PassManagerBuilder &Builder, llvm::legacy::PassManagerBase &PM) {
	PM.add(new PocPass());
}

// These constructors add our pass to a list of global extensions.
static RegisterStandardPasses clangtoolLoader_Ox(PassManagerBuilder::EP_OptimizerLast, loadPass);
static RegisterStandardPasses clangtoolLoader_O0(PassManagerBuilder::EP_EnabledOnOptLevel0, loadPass);
