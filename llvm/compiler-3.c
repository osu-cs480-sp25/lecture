#include <stdio.h>
#include <stdlib.h>

#include <llvm-c/Core.h> // Include LLVM core header for basic LLVM types and functions
#include <llvm-c/Analysis.h> // Include LLVM analysis header for module verification

int main()
{
    /*
        Generating IR code for the following C function:
        void arith_fn() {
            return (4.0 + 2.0) * 3.0;
        }
    */

    LLVMModuleRef module = LLVMModuleCreateWithName(
        "lecture.code.3" // Name of the module
    );
    LLVMBuilderRef builder = LLVMCreateBuilder(); // Create a new LLVM builder

    LLVMTypeRef arith_fn_sig = LLVMFunctionType(
        LLVMFloatType(), // Return type of the function (void)
        NULL, // No parameters for the function
        0, // Number of parameters (0 in this case)
        0 // Not a variadic function (0 means not variadic)
    );

    LLVMValueRef arith_fn_impl = LLVMAddFunction(
        module, // The module to which the function is added
        "arith_fn", // Name of the function
        arith_fn_sig // Function signature (type)
    );

    LLVMBasicBlockRef arith_fn_entry_blk = LLVMAppendBasicBlock(
        arith_fn_impl, // The function to which the block is added
        "arith_fn_entry" // Name of the block
    );

    LLVMPositionBuilderAtEnd( // Position the builder at the end of the entry block
        builder, // The builder to use for generating instructions
        arith_fn_entry_blk // The basic block where instructions will be inserted
    );

    LLVMValueRef constant4 = LLVMConstReal(LLVMFloatType(), 4.0); // Create a constant value of 4.0
    LLVMValueRef constant2 = LLVMConstReal(LLVMFloatType(), 2.0); // Create a constant value of 2.0
    LLVMValueRef sum = LLVMBuildFAdd( // Build an addition instruction
        builder, // The builder to use for generating instructions
        constant4, // The left-hand side operand (4.0)
        constant2, // The right-hand side operand (2.0)
        "sum" // Name of the result value (virtual register)
    );

    LLVMValueRef constant3 = LLVMConstReal(LLVMFloatType(), 3.0); // Create a constant value of 3.0
    LLVMValueRef product = LLVMBuildFMul( // Build a multiplication instruction
        builder, // The builder to use for generating instructions
        sum, // The left-hand side operand (4.0 + 2.0)
        constant3, // The right-hand side operand (3.0)
        "product" // Name of the result value (virtual register)
    );

    LLVMBuildRet( // Build a return instruction
        builder, // The builder to use for generating instructions
        product // The value to return (the result of the arithmetic expression)
    );

    LLVMVerifyModule( // Verify the module to ensure it is well-formed
        module, // Verify the module to ensure it is well-formed
        LLVMAbortProcessAction, // Action to take on verification failure (abort process)
        NULL // No additional context needed for verification
    );
    
    char* moduleString = LLVMPrintModuleToString // Print the module to a string
    (
        module // The module to print
    );
    printf("%s\n", moduleString); // Print the module to stdout

    LLVMDisposeBuilder(builder); // Dispose of the builder to free memory
    LLVMDisposeMessage(moduleString); // Free the string
    LLVMDisposeModule(module); // Dispose of the module to free memory
    return 0;
}