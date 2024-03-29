#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "support.h"
#include "wires.h"
#include "arithmetic.h"
#include "memories.h"
#include "alu.h"

#define HLT 0x0
#define NOP 0x1
#define MOV_RtoR 0x2
#define MOV_ItoR 0x3
#define MOV_RtoM 0x4
#define MOV_MtoR 0x5
#define ARITHMETIC 0x6
#define JCC 0x7
#define CALL 0x8
#define RET 0x9
#define PUSH 0xA
#define POP 0xB

#define REG_SP from_int(4)

int main(int argc, char* argv[]) {

    // Check command line parameters.
    if (argc < 2)
        error("missing name of programfile to simulate");

    trace_reader_p tracer = NULL;
    if (argc == 3) {
        tracer = create_trace_reader(argv[2]);
    }

    // Setup global state. 
    // Each cycle ends by capturing computations in global state.

    // Registerfile:
    // - we use separate read and write ports for sp updates. This can be optimized away.
    // - There are other options - the datapath diagram, for example, uses only two
    //   read ports and two write ports - one for writing results from the ALU, another
    //   for writing results from memory.
    mem_p regs = memory_create(16,3,2); // 8-byte cells, 3 readports, 2 writeport

    // Memory:
    // - Shared instruction and data memory. Two read ports for instruction fetch,
    //   one read and one write for data.
    mem_p mem = memory_create(1024,3,1); // 8-byte cells, 3 readports, 1 writeport
    memory_read_from_file(mem, argv[1]);
    // special registers
    val pc = from_int(0);
    conditions cc; cc.of = cc.sf = cc.zf = false;

    // a stop signal for stopping the simulation.
    bool stop = false;

    // We need the instruction number for trace validation
    int instruction_number = 0;

    while (!stop) { // for each cycle:

        ++ instruction_number;

        // Fetch next instruction
        val inst_word = memory_read_unaligned(mem, 0, 1, pc, true);

        /* DECODER */
        // decode - here we pick apart the instruction in major and minor opcode,
        // register specifiers and any immediate value. Next we identify all the
        // instructions and generate control signals:
        // For A2 you'll need to add generation of more control signals to control
        // the rest of the machine.
        // Some of the signals below are not used initially, but you'll have to
        // use them to finish A2. Since they are unused, gcc will generate warnings
        // for them, but they are put here on purpose to make your task easier.
        val major_op = pick_bits(4,4, inst_word);   // 0xAb, first op
        val minor_op = pick_bits(0,4, inst_word);   // 0xaB, second op

        // Set condition flag
        bool cc_flag = (eval_condition(cc, minor_op));

        // Set data movement / movq signals
        bool is_move_rr = is(MOV_RtoR, major_op);
        bool is_move_ir = is(MOV_ItoR, major_op);
        bool is_move_rm = is(MOV_RtoM, major_op);
        bool is_move_mr = is(MOV_MtoR, major_op);

        // Set arithmetic signal
        bool is_ari = (is(ARITHMETIC, major_op));
        bool is_ari_sub = (is(ARITHMETIC, major_op) && is(1, minor_op));
        bool is_ari_and = (is(ARITHMETIC, major_op) && is(2, minor_op));
        bool is_ari_xor = (is(ARITHMETIC, major_op) && is(3, minor_op));   
        bool is_cmpq  = (is(ARITHMETIC, major_op) && is(4, minor_op));

        // Set control flow signals
        bool is_jcc = (is(JCC, major_op));
        bool is_j3c = (is_jcc && !(is(0, major_op)));
        bool is_call = (is(CALL, major_op));
        bool is_ret = (is(RET, major_op));
        bool is_push = (is(PUSH, major_op));
        bool is_pop = (is(POP, major_op));
       
        // Set size of instruction byte according to function
        bool one_reg = (is_move_ir || is_push || is_pop);
        bool has_regs = (is_move_rr || is_ari || one_reg);
        bool has_mem = (is_move_ir || is_move_rm || is_move_mr);
        bool has_regs_ari = (is_ari_and || is_ari_xor || is_ari_sub);


        val size = or( use_if((!has_regs && !has_mem), from_int(1)), 
                   or( use_if( has_regs, from_int(2)),
                   or( use_if((is_jcc || is_call), from_int(5)),
                       use_if( has_mem,from_int(6)) )));
        
        // Set registers from instruction
        val reg_a = pick_bits(12,4, inst_word);
        val reg_b = pick_bits(8,4, inst_word);

        // Set immediate from instruction
        val imm_bytes = or( use_if(!has_mem, pick_bits(8, 32, inst_word)),
                            use_if( has_mem, pick_bits(16, 32, inst_word)));
        val imm = imm_bytes;
        val sign_extended_imm = sign_extend(31, imm);

        // Set next instruction
        val next_inst_pc = add(pc, size);

        // If hlt is detected, set stop to true. 
        stop = is(HLT, major_op);

    /* EXECUTE */
    // execute - for now, this is just reading out operands from their registers
    // For A2 you'll need to add components that implement the more complex
    // instructions. It's the place to use the ALU and read from memory.
        
        // Read values in registers
        val op_a = memory_read(regs, 0, reg_a, true);
        val op_b = memory_read(regs, 1, reg_b, true);
        
        val op_rsp = memory_read(regs, 2, REG_SP, (is_call || is_ret || is_push || is_pop));
        int offset = or(use_if((is_call || is_push),from_int(-8)),use_if((is_ret || is_pop),from_int(8))).val;
        val op_rsp_off = from_int(op_rsp.val + offset);

        // Read value in memory
        val mem_val = memory_read(mem, 1, op_rsp, is_ret || is_pop);

    // select result for register update
    // For A2 there'll be a lot more to choose from, once you've added use of
    // the ALU and loading from memory to the code above.

    // Arithmetic operations
    alu_execute_result ari_res = (alu_execute(minor_op, op_a, op_b));

    

    val datapath_mov = or(use_if( (is_move_rr || is_move_rm), op_a), use_if(is_move_ir, sign_extended_imm));
    val datapath_ari = ari_res.result;
    
    val datapath_result = or( use_if((cc_flag && (is_move_rr || is_move_ir || is_move_rm)), datapath_mov),
                            or( use_if(is_push, op_a),
                            or( use_if(is_call, next_inst_pc),
                            or( use_if(is_ret || is_pop, mem_val),
                                use_if(is_ari, datapath_ari)
                                                            ))));
 
    // pick result value and target register
    // For A2 you'll likely have to extend this section as there will be two
    // register updates for some instructions.
        val target_reg = or(use_if((!is_pop && !is_call), reg_b),
                            or(use_if(is_pop, reg_a),
                            use_if((is_call || is_push || is_ret), REG_SP)));
        val target_mem = or(use_if(!has_mem, from_int(0)),
                         or(use_if(has_mem, add(imm, op_b)),
                         or(use_if(is_pop, op_rsp),
                            use_if(is_call || is_ret || is_push, op_rsp_off))));

        bool reg_ops = (is_pop || is_move_rr || is_move_ir || (is_ari && !(is_cmpq)));
        bool reg_wr_enable = ((cc_flag || has_regs_ari) && (reg_ops));
        bool reg_sp_wr_enable = (cc_flag && (is_call || is_ret || is_push || is_pop));

        bool mem_ops = (is_move_rm || is_call || is_push);
        bool mem_wr_enable = ((cc_flag) && (mem_ops));

     // determine PC for next cycle. Right now we can only execute next in sequence.
    // For A2 you'll have to pick the right value for branches, call and return as
    // well.
        
        val next_pc = or( use_if( ((!(is_jcc) || !(cc_flag)) && (!is_call && !is_ret)), next_inst_pc),
                      or( use_if( (is_j3c && cc_flag), imm),
                      or( use_if( (is_jcc && !is_j3c), imm),
                      or( use_if(is_call, imm),
                          use_if(is_ret, datapath_result)
                          ))));


        // potentially pretty-print something to show progress before
        // ending cycle and overwriting state from start of cycle:
        // For A2, feel free to add more information you'd like for your own debugging
    
        /* DEBUG INFORMATION */
        // printf("mem_val: %llx ", mem_val.val);
        // printf("instruction_number: %d ", instruction_number);
        // printf("next_inst_pc: %llx ", next_inst_pc);
        // printf("next_pc: %llx ", next_pc);
        // printf("of %d ", cc.of);
        // printf("zf %d ", cc.zf);
        // printf("sf %d ", cc.sf);
        // printf(" reg_wr_enable %d ", reg_wr_enable);
        // printf(" reg_sp_wr_enable %d ", reg_sp_wr_enable);
        // printf(" mem_wr_enable %d ", mem_wr_enable);
        // printf(" has_regs_ari %d", has_regs_ari);
        // printf(" cc_flag %d ", cc_flag);
        // printf(" sign_extended_imm: %llx ", sign_extended_imm.val);
        // printf(" target_reg: %llx ", target_reg.val);
        // printf(" target_mem: %llx ", target_mem.val);
        // printf(" inst_word: %llx ", inst_word.val);
        // printf(" major_op: %llx ", major_op.val);
        // printf(" minor_op: %llx ", minor_op.val);
        // printf(" size: %llu ", size.val);
        // printf(" reg_a: %llu ", reg_a.val);
        // printf(" reg_b: %llu ", reg_b.val);
        // printf(" op_a: %llu ", op_a.val);
        // printf(" op_rsp: %llx ", op_rsp.val);
        // printf(" op_rsp_off: %llx ", op_rsp_off.val);
        // printf(" op_b: %llu ", op_b.val);
        // printf(" datapath_result: %llx ", datapath_result.val);
        // printf("Rsp: %llx ", (memory_read(regs, 0, REG_SP, true)));
        // printf("Mem: %llx ", (memory_read(mem, 0, (memory_read(regs, 0, REG_SP, true)), true)));
        // printf("is_call: %d ", is_call);

        /* PROGRESS PRETTY PRINTING */
        printf("%3llx : ", pc.val);
        for (int j=0; j<size.val; ++j) {
          unsigned int byte = (inst_word.val >> (8*j)) & 0xff;
            printf("%x ", byte);
        }
        if (reg_wr_enable) {
            printf("\t\tr%llu = %llx", target_reg.val, datapath_result.val);
        }
        if (reg_sp_wr_enable) {
            printf("\t\tr%llu = %llx", REG_SP.val, op_rsp_off.val);
        }
        if (mem_wr_enable) {
            printf("\t\t[%llx] = %llx", target_mem.val, datapath_result.val);
        }
        printf("\n");

        if ((tracer != NULL) & !stop) {
            // Validate values written to registers and memory against trace from
            // reference simulator. We do this *before* updating registers in order
            // to have the old register content available in case trace validation
            // fails. If so, the error function is called, and by setting a breakpoint
            // on it, all values are easy to inspect with a debugger.
            validate_pc_wr(tracer, instruction_number, next_pc);
            if (reg_wr_enable) {
                validate_reg_wr(tracer, instruction_number, target_reg, datapath_result);
            }
            if (reg_sp_wr_enable) {
                validate_reg_wr(tracer, instruction_number, REG_SP, op_rsp_off);
            }
            if (mem_wr_enable) {
                validate_mem_wr(tracer, instruction_number, target_mem, datapath_result);
            }
                
         // For A2 you'll need to add validation for the other register written
        // (for instructions which do write the other register)
        // AND you'll need to add a call to validate memory writes to check
        // movq %rX,D(%rY) once that is implemented. You can use validate_mem_wr
        // found in "support.h"
        }

        // store results at end of cycle
    // For A2 you need to add updating the condition code register and you'll
    // need to write to an additional register for some instructions and implement
    // writing to memory.
        pc = next_pc;
        cc = ari_res.cc;

        memory_write(regs, 0, target_reg, datapath_result, reg_wr_enable);
        memory_write(regs, 1, REG_SP, op_rsp_off, reg_sp_wr_enable);
        memory_write(mem, 1, target_mem, datapath_result, mem_wr_enable);
    }
    printf("Done\n");
}

