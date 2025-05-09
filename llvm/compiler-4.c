#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // Include standard boolean type header
#include <llvm-c/Core.h> // Include LLVM core header for basic LLVM types and functions
#include <llvm-c/Analysis.h> // Include LLVM analysis header for module verification

LLVMValueRef declare_variable(const char* name, bool firstBlock, LLVMBuilderRef builder) {
    LLVMValueRef allocated_memory = NULL; // Initialize the allocated memory to NULL
    if (firstBlock) {
        // Get the entry block of the function
        LLVMBasicBlockRef block = LLVMGetEntryBasicBlock(LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder)));
        
        // Create a new builder
        LLVMBuilderRef tempBuilder = LLVMCreateBuilder();

        // Get the first instruction of the block
        LLVMValueRef first_instruction = LLVMGetFirstInstruction(block);
    
        if (LLVMIsAInstruction(first_instruction)) {
            // Position the builder before the first instruction
            LLVMPositionBuilderBefore(tempBuilder, first_instruction);
        } else {
            // Position the builder at the end of the entry block
            LLVMPositionBuilderAtEnd(tempBuilder, block); 
        }

        // Build an alloca instruction to allocate memory for the type
        // The alloca instruction allocates memory on the stack for a variable
        allocated_memory = LLVMBuildAlloca(tempBuilder, LLVMFloatType(), name); 
        LLVMDisposeBuilder(tempBuilder); // Dispose of the temporary builder
    } else {
        allocated_memory = LLVMBuildAlloca(builder, LLVMFloatType(), name);
    }   
    return allocated_memory; // Return the allocated memory
}

LLVMValueRef assign_to_variable(
    const char* name, 
    LLVMValueRef value,
    LLVMBuilderRef builder
)
{
    // Check if the right-hand side operand is undefined
    if (LLVMIsUndef(value)) {
        fprintf(stderr, "Error: Undefined value encountered.\n");
        return LLVMGetUndef(LLVMFloatType()); // Return an undefined value for invalid operands
    }

    // We will use the name of the variable to create a new alloca instruction
    LLVMValueRef variable = declare_variable(name, true, builder); // Declare the variable using the name and builder

    // Build a store instruction to assign the value to the variable
    return LLVMBuildStore(
        builder, // The builder to use for generating instructions
        value, // The value to store in the variable
        variable // The variable where the value will be stored
    );
}

int main()
{
    /*
        Generating IR code for the following C function:
        float arith_fn() {
            float a = 4.0;
            return 4.0;
        }
    */

    LLVMModuleRef module = LLVMModuleCreateWithName(
        "lecture.code.4" // Name of the module
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
    LLVMValueRef assignment = assign_to_variable("a", constant4, builder); // Assign 4.0 to variable 'a'
    
    LLVMBuildRet( // Build a return instruction
         builder, // The builder to use for generating instructions
         constant4 // The value to return (the result of the arithmetic expression)
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