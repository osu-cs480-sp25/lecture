#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // Include standard boolean type header

#include <llvm-c/Core.h> // Include LLVM core header for basic LLVM types and functions
#include <llvm-c/Analysis.h> // Include LLVM analysis header for module verification

#include "hash.h" // Include the hash table header file

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
    struct hash* symbols,
    LLVMBuilderRef builder
)
{
    // Check if the right-hand side operand is undefined
    if (LLVMIsUndef(value)) {
        fprintf(stderr, "Error: Undefined value encountered.\n");
        return LLVMGetUndef(LLVMFloatType()); // Return an undefined value for invalid operands
    }

    // We will use the name of the variable to create a new alloca instruction
    LLVMValueRef variable = NULL; // Initialize the variable to NULL
    if (hash_contains(symbols, name)) {
        variable = hash_get(symbols, name); // Return the memory location of the variable
    } else {
        variable = declare_variable(name, true, builder); // Declare the variable using the name and builder
        hash_insert(symbols, name, variable); // Add the variable to the hash table
    }

    // Build a store instruction to assign the value to the variable
    LLVMValueRef store = LLVMBuildStore(
        builder, // The builder to use for generating instructions
        value, // The value to store in the variable
        variable // The variable where the value will be stored
    );

    return LLVMBuildLoad2(
        builder, /* The builder to use */
        LLVMFloatType(), /* The type of the variable */
        variable, /* The variable to load */
        name /* The name of the variable */
    );
}

int main()
{
    /*
        Generating IR code for the following C function:
        float arith_fn() {
            float a = 4.0 + 2.0;
            float b = 3.0;
            float c = a * b;
            return c;
        }
    */

    struct hash* symbols = hash_create(); // Create a new hash table for symbols

    LLVMModuleRef module = LLVMModuleCreateWithName(
        "lecture.code.9" // Name of the module
    );

    // Create a new LLVM builder
    LLVMBuilderRef builder = LLVMCreateBuilder(); 

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

    LLVMValueRef sum = LLVMBuildFAdd( // Build an addition instruction
        builder, // The builder to use for generating instructions
        LLVMConstReal(LLVMFloatType(), 4.0), // The left-hand side operand (4.0)
        LLVMConstReal(LLVMFloatType(), 2.0), // The right-hand side operand (2.0)
        "sum" // Name of the result value (virtual register)
    );

    LLVMValueRef variable_a = assign_to_variable("a", sum, symbols, builder);    
    LLVMValueRef variable_b = assign_to_variable("b", LLVMConstReal(LLVMFloatType(), 3.0), symbols, builder);

    LLVMValueRef product = LLVMBuildFMul( // Build a multiplication instruction
        builder, // The builder to use for generating instructions
        variable_a, // The left-hand side operand (4.0 + 2.0)
        variable_b, // The right-hand side operand (3.0)
        "product" // Name of the result value (virtual register)
    );

    LLVMValueRef variable_c = assign_to_variable("c", product, symbols, builder);

    LLVMBuildRet( // Build a return instruction
         builder, // The builder to use for generating instructions
         variable_c // The value to return (the result of the arithmetic expression)
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