#include <stdio.h>
#include <stdlib.h>

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>

#include "hash.h"

LLVMValueRef allocate_memory(const char* name, LLVMBasicBlockRef block)
{
    LLVMBuilderRef tempBuilder = LLVMCreateBuilder();
    LLVMValueRef first_instruction = LLVMGetFirstInstruction(block);
  
    if (LLVMIsAInstruction(first_instruction)) {
        LLVMPositionBuilderBefore(tempBuilder, first_instruction);
    } else {
        LLVMPositionBuilderAtEnd(tempBuilder, block); 
    }

    LLVMValueRef alloca = LLVMBuildAlloca(tempBuilder, LLVMFloatType(), name); 
    LLVMDisposeBuilder(tempBuilder);
    return alloca;
}

LLVMValueRef declare_variable(const char* name, LLVMBuilderRef builder) {
    LLVMBasicBlockRef function_entryBlock = LLVMGetEntryBasicBlock(LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder)));
    return allocate_memory(name, function_entryBlock);
}

LLVMValueRef assign(const char* name, LLVMValueRef value, struct hash* symbols, LLVMBuilderRef builder)
{
    LLVMValueRef mem_loc = NULL;
    if (hash_contains(symbols, name)) {
        mem_loc = hash_get(symbols, name);
    } else {
        mem_loc = declare_variable(name, builder);
        hash_insert(symbols, name, mem_loc);
    }
    LLVMValueRef store = LLVMBuildStore(builder, value, mem_loc);
    return mem_loc;
}

LLVMValueRef assign_and_get_variable(const char* name, LLVMValueRef value, struct hash* symbols, LLVMBuilderRef builder) {
    LLVMValueRef lhs = assign(name, value, symbols, builder);
    return LLVMBuildLoad2(builder, LLVMFloatType(), lhs, name);
}

LLVMValueRef get_variable(const char* name, struct hash* symbols, LLVMBuilderRef builder) {
    if (! hash_contains(symbols, name)) {
        fprintf(stderr, "Error: Variable '%s' not found.\n", name); // Print an error message if the variable is not found
        return LLVMGetUndef(LLVMFloatType());
    }
    return LLVMBuildLoad2(builder, LLVMFloatType(), hash_get(symbols, name), name);
}

LLVMValueRef constant(float value) {
    return  LLVMConstReal(LLVMFloatType(), value);
}

LLVMValueRef less_than(LLVMValueRef lhs, LLVMValueRef rhs, LLVMBuilderRef builder) {
    return LLVMBuildFCmp(builder, LLVMRealULT, lhs, rhs, "less_than");
}

LLVMValueRef arithmetic_operation(const char* operation, LLVMValueRef lhs, LLVMValueRef rhs, LLVMBuilderRef builder) {
    if (operation[0] == '+') {
        return LLVMBuildFAdd(builder, lhs, rhs, "sum");
    } else if (operation[0] == '-') {
        return LLVMBuildFSub(builder, lhs, rhs, "difference");
    } else if (operation[0] == '*') {
        return LLVMBuildFMul(builder, lhs, rhs, "product");
    } else if (operation[0] == '/') {
        return LLVMBuildFDiv(builder, lhs, rhs, "quotient");
    }
    return LLVMGetUndef(LLVMFloatType());
}

LLVMValueRef build_if_else(struct hash* symbols, LLVMBuilderRef builder) {
    LLVMValueRef variable_x = assign_and_get_variable("x", constant(3), symbols, builder);
    LLVMValueRef variable_y = assign_and_get_variable("y", constant(5), symbols, builder);
 
    LLVMValueRef condition = less_than(variable_x, constant(8), builder);
    LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));
 
    LLVMBasicBlockRef if_then_blk = LLVMAppendBasicBlock(current_function, "if.then");
    LLVMBasicBlockRef if_else_blk = LLVMAppendBasicBlock(current_function, "if.else");
    LLVMBasicBlockRef if_cont_blk = LLVMAppendBasicBlock(current_function, "if.continue");
    
    LLVMBuildCondBr(builder, condition, if_then_blk, if_else_blk);

    LLVMPositionBuilderAtEnd(builder, if_then_blk);
    LLVMValueRef then_value = arithmetic_operation("*", variable_x, variable_y, builder);
    assign("z", then_value, symbols, builder);
    LLVMBuildBr(builder, if_cont_blk);

    LLVMPositionBuilderAtEnd(builder, if_else_blk);
    LLVMValueRef else_value = arithmetic_operation("+", variable_x, variable_y, builder);
    assign("z", else_value, symbols, builder);
    LLVMBuildBr(builder, if_cont_blk);

    LLVMPositionBuilderAtEnd(builder, if_cont_blk);
    return LLVMBasicBlockAsValue(if_cont_blk);
}

int main()
{
    struct hash* symbols = hash_create();

    LLVMModuleRef module = LLVMModuleCreateWithName(
        "lecture.code.11"
    );

    LLVMBuilderRef builder = LLVMCreateBuilder(); 

    LLVMTypeRef arith_fn_sig = LLVMFunctionType(LLVMFloatType(),NULL,0,0);
    LLVMValueRef arith_fn = LLVMAddFunction(module, "arith_fn", arith_fn_sig);
    LLVMBasicBlockRef block = LLVMAppendBasicBlock(arith_fn, "block");
    LLVMPositionBuilderAtEnd(builder, block);

    build_if_else(symbols, builder);
    LLVMValueRef result = get_variable("z", symbols, builder);
    LLVMBuildRet(builder, result);

    LLVMVerifyModule(module, LLVMAbortProcessAction, NULL);

    char* moduleString = LLVMPrintModuleToString(module);
    printf("%s\n", moduleString);

    LLVMDisposeBuilder(builder);
    LLVMDisposeMessage(moduleString);
    LLVMDisposeModule(module);
    return 0;
}
