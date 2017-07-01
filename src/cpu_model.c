/*
 * cpu_model.c
 *
 *  Created on: Jun 24, 2017
 *      Author: morrisma
 */

#include "cpu_model.h"

void execute(CPU *p);
void decode(CPU *p);

void cpu_model(CPU *p)
{
	if(RST == p->op) {
		p->sm = RST1;
		p->op = IF;
	    p->ma = RST_VECTOR;
	    p->io = 0;
	    p->ip = p->ma;
	    p->xp = 0; p->yp = 0; p->ys = 0;
	    p->ki = 0; p->ir = 0;
	    p->ra = 0; p->rb = 0; p->cy = 0;
	    p->clr_ki = 0; p->ld_kil = 1; p->ld_kih = 0;
	} else {
		switch(p->sm) {
			case RST1 :
				p->sm = RST2;
				p->op = IF;
				p->ma = p->ma + 1;
				p->ip = p->ma;
				p->ki = (p->io & 0x00FF);
				p->clr_ki = 0; p->ld_kil = 0; p->ld_kih = 1;
				break;

			case RST2 :
				p->sm = DONE;
				p->op = IF;
				p->ki = ((p->io << 8) & 0xFF00) | (p->ki & 0x00FF);
				p->ma = p->ki;
				p->ip = p->ma;
				p->clr_ki = 1; p->ld_kil = 0; p->ld_kih = 0;
				break;

			case DONE :
				// execute previously fetched and decoded instruction
				execute(p);
			    // decode instruction using currently fetched value
			    decode(p);
				break;

			case LDA :
				p->sm = DONE;
				p->op = IF;
				p->ki = (p->ki & 0xFF00) | (p->io & 0x00FF);
				p->ma = p->ip + 1;
				p->ip = p->ma;
				p->clr_ki = 1; p->ld_kil = 0; p->ld_kih = 0;
				break;

			case STA :
				p->sm = DONE;
				p->op = IF;
				p->ma = p->ip + 1;
				p->ip = p->ma;
				p->clr_ki = 1; p->ld_kil = 0; p->ld_kih = 0;
				break;

			case LDYL :
				p->sm = LDYH;
				p->op = RD;
				p->ki = (p->ki & 0xFF00) | (p->io & 0x00FF);
				p->ma = p->ma + 1;
				p->clr_ki = 0; p->ld_kil = 0; p->ld_kih = 1;
				break;

			case LDYH :
				p->sm = DONE;
				p->op = IF;
				p->ki = ((p->io << 8) & 0xFF00) | (p->ki & 0x00FF);
				p->ma = p->ip + 1;
				p->ip = p->ma;
				p->clr_ki = 1; p->ld_kil = 0; p->ld_kih = 0;
				break;

			case STYL :
				p->sm = STYH;
				p->op = WR;
				p->ma = p->ma + 1;
				p->io = (p->yp >> 8) & 0x00FF;
				p->clr_ki = 0; p->ld_kil = 0; p->ld_kih = 1;
				break;

			case STYH :
				p->sm = DONE;
				p->op = IF;
				p->ma = p->ip + 1;
				p->ip = p->ma;
				p->clr_ki = 1; p->ld_kil = 0; p->ld_kih = 0;
				break;

			case RTS1 :
				p->sm = RTS2;
				p->op = RD;
				p->ki = p->io & 0x00FF;		// save low byte return address
				p->ma = p->xp;
				p->xp = p->xp + 1;			// increment xp
				p->clr_ki = 0; p->ld_kil = 0; p->ld_kih = 1;
				break;

			case RTS2 :
				p->sm = DONE;
				p->op = IF;
				p->ki = ((p->io << 8) & 0xFF00) | (p->ki & 0x00FF);
				p->ma = p->ki + 1;
				p->xp = p->xp + 1;			// restore xp
				p->clr_ki = 1; p->ld_kil = 0; p->ld_kih = 0;
				break;

			case JSR1 :
				p->sm = JSR2;
				p->op = WR;
				p->ma = p->xp - 1;			// decrement xp
				p->io = p->ip & 0x00FF;		// push low byte return address
				p->xp = p->ma;
				p->clr_ki = 0; p->ld_kil = 0; p->ld_kih = 1;
				break;

			case JSR2 :
				p->sm = DONE;
				p->op = IF;
	            p->ma = p->ip + p->ki + 1;	// fetch next instruction
	            p->ip = p->ma;
				p->clr_ki = 1; p->ld_kil = 0; p->ld_kih = 0;
				break;
		} // switch
	} // if-else
} // cpu_model

void execute(CPU *p)
{
    // Execute Current instruction

    switch(p->ir) {
        case PFX :
            break;

        case NFX :
            p->ki = ~p->ki;   // complement ki
            break;

        case INV :
            p->ma = p->ip;    // error - unexpected instruction opcode
            p->ip = p->ma;
            break;

        case LDK :
        case LDL :
        case LDN :
            p->ra = (p->ki & 0x00FF); // write ki into cpu reg A
            break;

        case STL :
        case STN :
            break;

        case LDY :
        	p->ys = p->yp;		// push yp into ys
        	p->yp = p->ki;      // load yp from ki
            break;

        case STY :
        	p->yp = p->ys;		// pop ys into yp
        	break;

        case BNE :
        case BPL :
        case BNC :
            break;

        case ADJ :
            p->xp += p->ki;   // adjust workspace pointer
            break;

        case RTS :
        case JSR :
            break;

        // Indirect Instructions - currently all are single cycle.

        case SWP :
            {
                unsigned short tmp = p->yp;

                p->yp = p->ys;
                p->ys = tmp;
            }
            break;

        case XAB :
            {
                unsigned char tmp = p->ra;

                p->ra = p->rb;
                p->rb = tmp;
            }
            break;

        case XCH :
            {
                unsigned short tmp = p->xp;

                p->xp = ((p->ra << 8) | (p->rb &0x00FF));
                p->ra = ((tmp >> 8) & 0x00FF);
                p->rb = (tmp & 0x00FF);
            }
            break;

        case RFU :
            break;

        case CLC :
            p->cy = 0;
            break;

        case SEC :
            p->cy = 1;
            break;

        case ADC :
            {
                unsigned short tmp = (unsigned short) p->ra;

                tmp += (unsigned short) p->rb;
                tmp += (unsigned short) (p->cy & 1);

                p->ra = (unsigned char) (tmp & 0x00FF);
                p->cy = ((tmp > 255) ? 1 : 0);
            }
            break;

        case SBC :
            {
                unsigned short tmp = (unsigned short) p->ra;

                tmp += (unsigned short) ((~p->rb) & 0x00FF);
                tmp += (unsigned short) (p->cy & 1);

                p->ra = (unsigned char) (tmp & 0x00FF);
                p->cy = ((tmp > 255) ? 1 : 0);
            }
            break;

        case ROL :
            {
                char tmp = (p->ra & 0x80);

                p->ra = (p->ra << 1) | (p->cy & 1);
                p->cy = ((tmp >> 7) & 1);
            }
            break;

        case ASL :
            {
                char tmp = (p->ra & 0x80);

                p->ra = ((p->ra << 1) & 0xFE);
                p->cy = ((tmp >> 7) & 1);
            }
            break;

        case ROR :
            {
                char tmp = (p->ra & 0x01);

                p->ra = (p->ra >> 1) | ((p->cy & 1) << 7);
                p->cy = tmp;
            }
            break;

        case ASR :
            {
                char tmp = (p->ra & 0x01);

                p->ra = ((p->ra & 0x80) ? ((p->ra >> 1) | 0x80)
                                        :  (p->ra >> 1)        );
                p->cy = tmp;
            }
            break;

        case CPL :
            p->ra = (~p->ra);
            break;

        case AND :
            p->ra = (p->ra & p->rb);
            break;

        case ORL :
            p->ra = (p->ra | p->rb);
            break;

        case XOR :
            p->ra = (p->ra ^ p->rb);
            break;
    }
}

void decode(CPU *p)
{
	// update ki during instruction fetch

	if(p->clr_ki) {
        p->ki = (p->io & 0x0F);
    } else {
        p->ki = (p->ki << 4) | (p->io & 0x0F);
    }

    // update ir

    if(EXE == (p->io & 0xF0)) {
        p->ir = 0x10 | (p->io & 0x0F);  // defined indirect instructions
    } else {
        p->ir = ((p->io >> 4) & 0x0F);
    }

    // Decode instruction and perform operations required by instruction

    switch(p->ir) {
        case PFX :
        case NFX :
        	p->sm = DONE;
        	p->op = IF;
        	p->ma = p->ip + 1;    	// set p->ma : fetch next instruction
            p->ip = p->ma;
			p->clr_ki = 0; p->ld_kil = 0; p->ld_kih = 0;
            break;

        case LDK :
        	p->sm = DONE;
        	p->op = IF;
        	p->ma = p->ip + 1;    	// set p->ma : fetch next instruction
            p->ip = p->ma;
			p->clr_ki = 1; p->ld_kil = 0; p->ld_kih = 0;
            break;

        case INV :
        	p->sm = DONE;
        	p->op = IF;
            p->ma = p->ip;    // error - unexpected instruction opcode
            p->ip = p->ma;
            break;

        case LDL :
        	p->sm = LDA;
        	p->op = RD;
            p->ma = p->xp + p->ki;   // calculate local
            p->clr_ki = 0; p->ld_kil = 1; p->ld_kih = 0;
            break;

        case LDN :
        	p->sm = LDA;
        	p->op = RD;
            p->ma = p->yp + p->ki;   // calculate non-local
            p->clr_ki = 0; p->ld_kil = 1; p->ld_kih = 0;
          break;

        case STL :
        	p->sm = STA;
        	p->op = WR;
            p->ma = p->xp + p->ki;   // calculate local
            p->io = p->ra;
            p->clr_ki = 0; p->ld_kil = 1; p->ld_kih = 0;
            break;

        case STN :
        	p->sm = STA;
        	p->op = WR;
            p->ma = p->yp + p->ki;   // calculate non-local
            p->io = p->ra;
            p->clr_ki = 0; p->ld_kil = 1; p->ld_kih = 0;
            break;

        case LDY :
        	p->sm = LDYL;
        	p->op = RD;
            p->ma = p->xp + p->ki;   // calculate local
            p->clr_ki = 0; p->ld_kil = 1; p->ld_kih = 0;
           break;

        case STY :
        	p->sm = STYL;
        	p->op = WR;
            p->ma = p->xp + p->ki;   // calculate local
            p->io = (p->yp & 0x00FF);
            p->clr_ki = 0; p->ld_kil = 1; p->ld_kih = 0;
           break;

        case BNE :
        	p->sm = DONE;
        	p->op = IF;
            if(0 != p->ra) {
                p->ma  = p->ip + p->ki + 1;		// branch
            } else {
                p->ma = p->ip + 1;  			// increment ip
            }
            p->ip = p->ma;
            p->clr_ki = 1; p->ld_kil = 0; p->ld_kih = 0;
            break;

        case BPL :
        	p->sm = DONE;
        	p->op = IF;
            if(0 == (p->ra & 0x80)) {
                p->ma = p->ip + p->ki + 1;   	// branch
            } else {
                p->ma = p->ip + 1;           	// increment ip
            }
            p->ip = p->ma;
            p->clr_ki = 1; p->ld_kil = 0; p->ld_kih = 0;
            break;

        case BNC :
        	p->sm = DONE;
        	p->op = IF;
            if(0 == p->cy) {
                p->ma = p->ip + p->ki + 1;   	// branch
            } else {
                p->ma = p->ip + 1;            	// increment ip
            }
            p->ip = p->ma;
            p->clr_ki = 1; p->ld_kil = 0; p->ld_kih = 0;
            break;

        case ADJ :
        	p->sm = DONE;
        	p->op = IF;
            p->ma = p->ip + 1;    // set p->ma : fetch next instruction
            p->ip = p->ma;
            p->clr_ki = 1; p->ld_kil = 0; p->ld_kih = 0;
            break;

        case RTS :
        	p->sm = RTS1;
        	p->op = RD;
            p->ma = p->xp + p->ki;  // pop low byte of return address
            p->xp = p->ma + 1;		// increment workspace pointer for pop
            p->clr_ki = 0; p->ld_kil = 1; p->ld_kih = 0;
            break;

        case JSR :
        	p->sm = JSR1;
        	p->op = WR;
            p->ma = p->xp - 1;    			// push high byte of return address
            p->xp = p->ma;
            p->io = (p->ip >> 8) & 0x00FF;	// output high byte of ip
            p->clr_ki = 0; p->ld_kil = 1; p->ld_kih = 0;
            break;

        // Indirect Instructions - currently all are single cycle.

        case SWP :
        case XAB :
        case XCH :
        case RFU :
        case CLC :
        case SEC :
        case ADC :
        case SBC :
        case ROL :
        case ASL :
        case ROR :
        case ASR :
        case CPL :
        case AND :
        case ORL :
        case XOR :
        	p->sm = DONE;
        	p->op = IF;
            p->ma = p->ip + 1;
            p->ip = p->ma;
            p->clr_ki = 1; p->ld_kil = 0; p->ld_kih = 0;
            break;

    }
}

