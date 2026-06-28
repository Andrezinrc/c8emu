#define TRACE_CPU(conf, cpu, name, op) \
    do { \
        if ((conf)->trace) { \
            printf("[0x%03X] %-10s (0x%04X) | V%X=%02X, V%X=%02X, I=%04X\n", \
                (cpu)->PC, \
                (name), \
                (op), \
                OP_X(op), (cpu)->V[OP_X(op)], \
                OP_Y(op), (cpu)->V[OP_Y(op)], \
                (cpu)->I); \
        } \
    } while (0)
