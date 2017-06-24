/*
 * cpu_model.c
 *
 *  Created on: Jun 24, 2017
 *      Author: morrisma
 */

#include "MiniCPU.h"

void cpu_model(CPU *p, unsigned char ram[])
{
    // update ki during instruction fetch

    if(p->clr_ki) {
            p->ki = (p->io & 0x0F);
    } else {
        p->ki = (p->ki << 4) | (p->io & 0x0F);
    }

    // update ir

    if(EXE == (p->io & 0xF0)) {
        p->ir = 16 + (p->io & 0x0F);  // defined indirect instructions
    } else {
        p->ir = ((p->io >> 4) & 0x0F);
    }

    // Decode instruction and perform operations required by instruction

    switch(p->ir) {
        case PFX :
            p->clr_ki = 0;     // do not clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ma = p->ip + 1;    // set p->ma : fetch next instruction
            p->ip = p->ma;
            break;

        case NFX :
            p->clr_ki = 0;     // do not clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ki = ~p->ki;   // complement ki

            p->ma = p->ip + 1;    // set p->ma : fetch next instruction
            p->ip = p->ma;
            break;

        case INV :
            p->clr_ki = 0;     // do not clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ma = p->ip;    // error - unexpected instruction opcode
            p->ip = p->ma;
            break;

        case LDK :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ra = (p->ki & 0x00FF); // write ki into cpu reg A

            p->ma = p->ip + 1;    // set p->ma : fetch next instruction
            p->ip = p->ma;
            break;

        case LDL :
            p->clr_ki = 0;     // do not clear ki
            p->ld_kil = 1;     // load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ma = p->xp + p->ki;   // calculate local
            p->io = ram[p->ma % DML]; // read data
            p->ki = (p->ki & 0xFF00) + p->io;

            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ra = (p->ki & 0x00FF);

            p->ma = p->ip + 1;    // set p->ma : fetch next instruction
            p->ip = p->ma;
            break;

        case LDN :
            p->clr_ki = 0;     // do not clear ki
            p->ld_kil = 1;     // load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ma = p->yp + p->ki;   // calculate non-local
            p->io = ram[p->ma % DML]; // read data
            p->ki = (p->ki & 0xFF00) + p->io;

            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ra = (p->ki & 0x00FF);

            p->ma = p->ip + 1;    // set p->ma : fetch next instruction
            p->ip = p->ma;
            break;

        case STL :
            p->clr_ki = 0;     // do not clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ma = p->xp + p->ki;   // calculate local
            p->io = p->ra;

            ram[p->ma % DML] = p->io; // write data

            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do no load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ma = p->ip + 1;    // set p->ma : fetch next instruction
            p->ip = p->ma;
            break;

        case STN :
            p->clr_ki = 0;     // do not clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ma = p->yp + p->ki;   // calculate non-local
            p->io = p->ra;

            ram[p->ma % DML] = p->io; // write data

            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ma = p->ip + 1;    // set p->ma : fetch next instruction
            p->ip = p->ma;
            break;

        case LDY :
            p->clr_ki = 0;     // do not clear ki
            p->ld_kil = 1;     // load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ma = p->xp + p->ki;       // calculate local
            p->io = ram[p->ma % DML];     // read data low
            p->ki = (p->ki & 0xFF00) + p->io;

            p->clr_ki = 0;     // do not clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 1;     // load ki[15:8]

            p->ma++;           // increment memory address
            p->io = ram[p->ma % DML];     // read data high
            p->ki = (p->ki & 0x00FF) + (p->io << 8);

            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->yp = p->ki;    // load yp from ki

            p->ma = p->ip + 1;    // set p->ma : fetch next instruction
            p->ip = p->ma;
            break;

        case STY :
            p->clr_ki = 0;     // do not clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ma  = p->xp + p->ki;          // calculate local
            p->io = (p->yp & 0x00FF);         // output data low
            ram[p->ma % DML] = p->io;         // write memory

            p->clr_ki = 0;     // do not clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ma++;                           // increment mem address
            p->io = ((p->yp >> 8) & 0x00FF);  // output data high
            ram[p->ma % DML] = p->io;         // write memory

            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ma  = p->ip + 1;   // set p->ma : fetch next instruction
            p->ip  = p->ma;
            break;

        case BNE :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            if(p->ra != 0) {
                p->ma  = p->ip + p->ki + 1;  // branch
            } else {
                p->ma = p->ip + 1;            // increment ip
            }
            p->ip = p->ma;
            break;

        case BPL :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            if((p->ra & 0x80) == 0) {
                p->ma = p->ip + p->ki + 1;   // branch
            } else {
                p->ma = p->ip + 1;            // increment ip
            }
            p->ip = p->ma;
            break;

        case BNC :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            if(p->cy == 0) {
                p->ma = p->ip + p->ki + 1;   // branch
            } else {
                p->ma = p->ip + 1;            // increment ip
            }
            p->ip = p->ma;
            break;

        case ADJ :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->xp += p->ki;   // adjust workspace pointer

            p->ma = p->ip + 1;    // set p->ma : fetch next instruction
            p->ip = p->ma;
            break;

        case RTS :
            p->clr_ki = 0;     // do not clear ki
            p->ld_kil = 1;     // load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ma = p->xp + p->ki;       // calculate local
            p->xp = p->ma;                // adjust xp
            p->io = ram[p->ma % DML];     // read data low

            p->clr_ki = 0;     // do not clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 1;     // load ki[15:8]

            p->ma++;                       // increment memory address
            p->xp++;                       // increment xp
            p->io = ram[p->ma % DML];     // read data high

            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->xp++;           // increment xp (restore previous workspace)

            p->ma = p->ki + 1;    // set p->ma : fetch next instruction
            p->ip = p->ma;
            break;

        case JSR :
            p->clr_ki = 0;     // do not clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ma = p->xp - 1;
            p->xp--;
            p->io = ((p->ip >> 8) & 0x00FF);  // write ip high
            ram[p->ma % DML] = p->io;

            p->clr_ki = 0;     // do not clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ma = p->xp - 1;
            p->xp--;
            ram[p->ma % DML] = p->io;
            p->io = (p->ip & 0x00FF);         // write ip low

            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ma = p->ip + p->ki + 1; // set p->ma : fetch next
            p->ip = p->ma;
            break;

        // Indirect Instructions - currently all are single cycle.

/*
        case POP :
            p->ip += 1;

            p->ra = p->rb;

            p->ki = 0;
            p->ma = p->ip;
            break;
*/

        case SWP :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            {
                short tmp = p->yp;
                p->yp = p->ys;
                p->ys = tmp;
            }

            p->ma = p->ip + 1;
            p->ip = p->ma;
            break;

        case XAB :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            {
                char tmp = p->ra;

                p->ra = p->rb;
                p->rb = tmp;
            }

            p->ma = p->ip + 1;
            p->ip = p->ma;
            break;

        case XCH :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            {
                short tmp = p->xp;

                p->xp = ((p->ra << 8) + (p->rb &0x00FF));
                p->ra = ((tmp >> 8) & 0x00FF);
                p->rb = (tmp & 0x00FF);
            }

            p->ma = p->ip + 1;
            p->ip = p->ma;
            break;

        case LDA :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ra = (p->ki & 0xFF);

            p->ma = p->ip + 1;
            p->ip = p->ma;
            break;

        case CLC :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->cy = 0;

            p->ma = p->ip + 1;
            p->ip = p->ma;
            break;

        case SEC :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->cy = 1;

            p->ma = p->ip + 1;
            p->ip = p->ma;
            break;

        case ADC :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            {
                unsigned short tmp = (unsigned short) p->ra;

                tmp += (unsigned short) p->rb;
                tmp += (unsigned short) (p->cy & 1);

                p->ra = (char) (tmp & 0x00FF);
                p->cy = ((tmp > 255) ? 1 : 0);
            }

            p->ma = p->ip + 1;
            p->ip = p->ma;
            break;

        case SBC :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            {
                unsigned short tmp = (unsigned short) p->ra;

                tmp += (unsigned short) (~p->rb & 0x00FF);
                tmp += (unsigned short) (p->cy & 1);

                p->ra = (char) (tmp & 0x00FF);
                p->cy = ((tmp > 255) ? 1 : 0);
            }

            p->ma = p->ip + 1;
            p->ip = p->ma;
            break;

        case ROL :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            {
                char tmp = (p->ra & 0x80);

                p->ra = (p->ra << 1) + (p->cy & 1);
                p->cy = ((tmp >> 7) & 1);
            }

            p->ma = p->ip + 1;
            p->ip = p->ma;
            break;

        case ASL :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            {
                char tmp = (p->ra & 0x80);

                p->ra = ((p->ra << 1) & 0xFE);
                p->cy = ((tmp >> 7) & 1);
            }

            p->ma = p->ip + 1;
            p->ip = p->ma;
            break;

        case ROR :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            {
                char tmp = (p->ra & 1);

                p->ra = (p->ra >> 1) + ((p->cy & 1) << 7);
                p->cy = tmp;
            }

            p->ma = p->ip + 1;
            p->ip = p->ma;
            break;

        case ASR :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            {
                char tmp = (p->ra & 1);

                p->ra = ((p->ra & 0x80) ? ((p->ra >> 1) | 0x80)
                                          :  (p->ra >> 1)        );
                p->cy = tmp;
            }

            p->ma = p->ip + 1;
            p->ip = p->ma;
            break;

        case CPL :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ra = (~p->ra);

            p->ma = p->ip + 1;
            p->ip = p->ma;
            break;

        case AND :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ra = (p->ra & p->rb);

            p->ma = p->ip + 1;
            p->ip = p->ma;
            break;

        case ORL :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ra = (p->ra | p->rb);

            p->ma = p->ip + 1;
            p->ip = p->ma;
            break;

        case XOR :
            p->clr_ki = 1;     // clear ki
            p->ld_kil = 0;     // do not load ki[7:0]
            p->ld_kih = 0;     // do not load ki[15:8]

            p->ra = (p->ra ^ p->rb);

            p->ma = p->ip + 1;
            p->ip = p->ma;
            break;
    }
}

