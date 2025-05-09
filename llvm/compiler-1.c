#include <stdio.h>
#include <stdlib.h>

#include <llvm-c/Core.h> // Include LLVM core header for basic LLVM types and functions
#include <llvm-c/Analysis.h> // Include LLVM analysis header for module verification

int main()
{
    /*
        Generating IR code for the following C function:
        void void_fn() {
            return;
        }
    */

    // Create a new LLVM module and builder
    LLVMModuleRef module = LLVMModuleCreateWithName(
        "lecture.code.1" // Name of the module
    );
    LLVMBuilderRef builder = LLVMCreateBuilder(); // Create a new LLVM builder

    LLVMTypeRef void_fn_sig = LLVMFunctionType(
        LLVMVoidType(), // Return type of the function (void)
        NULL, // No parameters for the function
        0, // Number of parameters (0 in this case)
        0 // Not a variadic function (0 means not variadic)
    );

    LLVMValueRef void_fn_impl = LLVMAddFunction(
        module, // The module to which the function is added
        "void_fn", // Name of the function
        void_fn_sig // Function signature (type)
    );

    LLVMBasicBlockRef void_fn_entry_blk = LLVMAppendBasicBlock(
        void_fn_impl, // The function to which the block is added
        "void_fn_entry" // Name of the block
    );

    LLVMPositionBuilderAtEnd( // Position the builder at the end of the entry block
        builder, // The builder to use for generating instructions
        void_fn_entry_blk // The basic block where instructions will be inserted
    );

    LLVMBuildRetVoid(builder); // Build a return instruction at the end of the entry block

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