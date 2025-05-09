#include <stdio.h>
#include <stdlib.h>

#include <llvm-c/Core.h> // Include LLVM core header for basic LLVM types and functions
#include <llvm-c/Analysis.h> // Include LLVM analysis header for module verification

#include "hash.h" // Include the hash table header file

LLVMValueRef allocate_memory(const char* name, LLVMBasicBlockRef block)
{
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
    LLVMValueRef alloca = LLVMBuildAlloca(tempBuilder, LLVMFloatType(), name); 
    LLVMDisposeBuilder(tempBuilder); // Dispose of the temporary builder
    return alloca;
}

LLVMValueRef declare_variable(const char* name, LLVMBuilderRef builder) {
    // Get the entry block of the function
    LLVMBasicBlockRef function_entryBlock = LLVMGetEntryBasicBlock(LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder)));
    
    // Call the allocate_memory function to allocate memory
    LLVMValueRef allocated_memory = allocate_memory(name, function_entryBlock);

    // Return the allocated memory
    return allocated_memory; 
}

LLVMValueRef assign_to_variable(
    const char* name, 
    LLVMValueRef value,
    struct hash* symbols,
    LLVMBuilderRef builder
)
{
    // For now, assume we have not previously declared the variable
    // We will use the name of the variable to create a new alloca instruction
    LLVMValueRef variable = NULL; // Initialize the variable to NULL
    if (hash_contains(symbols, name)) {
        variable = hash_get(symbols, name); // Return the memory location of the variable
    } else {
        variable = declare_variable(name, builder); // Declare the variable using the name and builder
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
        hash_get(symbols, name), /* The variable to load */
        name /* The name of the variable */
    );
}

LLVMValueRef build_constant(float value) {
    LLVMValueRef constant = LLVMConstReal(LLVMFloatType(), value); // Create a constant value of type float
    return constant; // Return the constant value
}

LLVMValueRef build_mem_alloc(const char* name, LLVMBuilderRef builder) {
    LLVMBasicBlockRef block = LLVMGetInsertBlock(builder); // Get the current basic block
    LLVMValueRef function = LLVMGetBasicBlockParent(block); // Get the parent function of the block
    LLVMBasicBlockRef function_entryBlock = LLVMGetEntryBasicBlock(function); // Get the entry block of the function
    LLVMValueRef first_instruction = LLVMGetFirstInstruction(function_entryBlock); // Get the first instruction of the entry block 
  
    LLVMBuilderRef tempBuilder = LLVMCreateBuilder(); // Create a new builder
    if (LLVMIsAInstruction(first_instruction)) {
        LLVMPositionBuilderBefore(tempBuilder, first_instruction); // Position the builder before the first instruction
    } else {
        LLVMPositionBuilderAtEnd(tempBuilder, function_entryBlock); // Position the builder at the end of the entry block
    }
    LLVMValueRef mem_alloc = LLVMBuildAlloca(tempBuilder, LLVMFloatType(), name); // Build an alloca instruction to allocate memory for the type
    LLVMDisposeBuilder(tempBuilder); // Dispose of the temporary builder
    
    return mem_alloc;
}

LLVMValueRef build_binary_operation(LLVMBuilderRef builder,
    LLVMValueRef lhs, LLVMValueRef rhs, const char operation) {
   // Check if the operands are undefined
  if (LLVMIsUndef(lhs) || LLVMIsUndef(rhs)) {
       fprintf(stderr, "Error: Undefined value encountered.\n");
       return LLVMGetUndef(LLVMFloatType()); // Return an undefined value for invalid operands
   }

   // Check the operation and create the corresponding LLVM instruction
   switch (operation) {
        case '+':
            return LLVMBuildFAdd(
                builder, /* The builder to use */
                lhs, /* The left-hand side operand */
                rhs, /* The right-hand side operand */
                "sum" /* Name of the register for the virtual result value */
            );
        case '-':
            return LLVMBuildFSub(
                builder, /* The builder to use */
                lhs, /* The left-hand side operand */
                rhs, /* The right-hand side operand */
                "difference" /* Name of the result value */
            );
        case '*':   
            return LLVMBuildFMul(
                builder, /* The builder to use */
                lhs, /* The left-hand side operand */
                rhs, /* The right-hand side operand */
                "product" /* Name of the result value */
            );    
        case '/':
            return LLVMBuildFDiv(
                builder, /* The builder to use */
                lhs, /* The left-hand side operand */
                rhs, /* The right-hand side operand */
                "quotient" /* Name of the result value */
            );
        case '<':
            LLVMValueRef lt_result = LLVMBuildFCmp(
                builder, /* The builder to use */
                LLVMRealULT, /* The comparison predicate */
                lhs, /* The left-hand side operand */
                rhs, /* The right-hand side operand */
                "less_than" /* Name of the result value */
            );
            // cast lt_result to a float type
            //lt_result = LLVMBuildZExt(builder, lt_result, LLVMFloatType(), "lt_result"); // Cast the result to a float type
            return LLVMBuildUIToFP(builder, lt_result, LLVMFloatType(), "cast_result"); // Cast the result to a float type
        default:    
            fprintf(stderr, "Unknown operation: %c\n", operation);
            return LLVMGetUndef(LLVMFloatType()); // Return an undefined value for unknown operations
   }
}

LLVMValueRef build_assignment (
    LLVMBuilderRef builder,
    const char* name, // Left-hand side operand
    LLVMValueRef exp, // Right-hand side operand
    struct hash* symbols // Hash table for symbol management   
) 
{
    // Check if the right-hand side operand is undefined
    if (LLVMIsUndef(exp)) {
        fprintf(stderr, "Error: Undefined value encountered.\n");
        return LLVMGetUndef(LLVMFloatType()); // Return an undefined value for invalid operands
    }

    if (!hash_contains(symbols, name)) {
        LLVMValueRef mem_alloc = build_mem_alloc(name, builder); // Allocate memory for the variable if it doesn't exist in the hash table  
        hash_insert(symbols, name, mem_alloc); // Add the variable to the hash table
    }

    // Create a store instruction to assign the value of rhs to lhs
    LLVMValueRef store = LLVMBuildStore(builder, exp, hash_get(symbols, name)); // Build a store instruction to assign the value of rhs to lhs

    return store; // Return the store instruction   
}

LLVMValueRef build_variable(
    LLVMBuilderRef builder,
    struct hash* symbols, // Hash table for symbol management   
    const char* name // Left-hand side operand
) 
{
    // Check if the variable exists in the hash table
    if (! hash_contains(symbols, name)) {
        fprintf(stderr, "Error: Variable '%s' not found.\n", name); // Print an error message if the variable is not found
        return LLVMGetUndef(LLVMFloatType());
    }
    return LLVMBuildLoad2(
        builder, /* The builder to use */
        LLVMFloatType(), /* The type of the variable */
        hash_get(symbols, name), /* The variable to load */
        name /* The name of the variable */
    );
}

LLVMValueRef build_if_else(struct hash* symbols, LLVMBuilderRef builder) {
    build_assignment(builder, "x", build_constant(3), symbols); // Assign a constant value to variable "a"
    build_assignment(builder, "y", build_constant(5), symbols); // Assign a constant value to variable "b"
 
    LLVMValueRef condition = build_binary_operation(
        builder, 
        build_variable(builder, symbols, "x"),
        build_constant(8),
        '<'
    );
    
    condition = LLVMBuildFCmp(
        builder, /* The builder to use */
        LLVMRealONE, /* The comparison predicate */
        condition, /* The left-hand side operand */
        build_constant(0), /* The right-hand side operand */
        "cond_results" /* Name of the result value */
    );

    LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder); // Get the current basic block
    LLVMValueRef current_function = LLVMGetBasicBlockParent(current_block); // Get the parent function of the block
 
    LLVMBasicBlockRef if_then_blk = LLVMAppendBasicBlock(
        /* The function */ current_function, 
        /* Name of the block*/ "if.then"
    );

    LLVMBasicBlockRef if_else_blk = LLVMAppendBasicBlock(
        /* The function */ current_function, 
        /* Name of the block*/ "if.else"
    );

    LLVMBasicBlockRef if_cont_blk = LLVMAppendBasicBlock(
        /* The function */ current_function, 
        /* Name of the block*/ "if.continue"
    );
    
    // Create a conditional branch based on the condition
    LLVMBuildCondBr(builder, condition, if_then_blk, if_else_blk); // Build a conditional branch instruction

    LLVMPositionBuilderAtEnd(builder, if_then_blk); // Position the builder at the end of the then block
    LLVMValueRef then_value = build_binary_operation(
        builder, 
        build_variable(builder, symbols, "x"),
        build_variable(builder, symbols, "y"),
        '*'
    );
    LLVMValueRef then_assign = build_assignment(builder, "z", then_value, symbols);
    LLVMBuildBr(builder, if_cont_blk); // Build an unconditional branch to the continue block

    LLVMPositionBuilderAtEnd(builder, if_else_blk); // Position the builder at the end of the then block
    LLVMValueRef else_value = build_binary_operation(
        builder, 
        build_variable(builder, symbols, "x"),
        build_variable(builder, symbols, "y"),
        '+'
    );
    LLVMValueRef else_assign = build_assignment(builder, "z", else_value, symbols);
    LLVMBuildBr(builder, if_cont_blk); // Build an unconditional branch to the continue block

    LLVMPositionBuilderAtEnd(builder, if_cont_blk); // Position the builder at the end of the continue block
    return LLVMBasicBlockAsValue(if_cont_blk); // Return the continue block as the result of the function
}

int main()
{
    struct hash* symbols = hash_create(); // Create a new hash table for symbols

    LLVMModuleRef module = LLVMModuleCreateWithName(
        "lecture.code.11" // Name of the module
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

    LLVMValueRef if_else = build_if_else(symbols, builder); // Build an if-else statement using the builder
    LLVMBuildRet(builder, build_variable(builder, symbols, "z")); // Build a return instruction at the end of the entry block

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