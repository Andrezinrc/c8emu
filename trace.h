#define DEBUG_MODE

#ifdef DEBUG_MODE
    #define TRACE_CPU(cpu, name, op) \
        printf("[0x%03X] %-10s (0x%04X) | V%X=%02X, V%X=%02X, I=%04X\n", \
            cpu->PC, \
            name, \
            op, \
            OP_X(op), cpu->V[OP_X(op)], \
            OP_Y(op), cpu->V[OP_Y(op)], \
            cpu->I)
#else
    #define TRACE_CPU(cpu, name, op) do {} while (0)
#endif
