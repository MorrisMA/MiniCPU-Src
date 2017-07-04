/*
 ============================================================================
 Name        : MiniCPU.c
 Author      : Michael A. Morris
 Version     :
 Copyright   : Copyright 2016 - GPLv3
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "MiniCPU.h"
#include "cpu_model.h"

////////////////////////////////////////////////////////////////////////////////

CPU cpu;

unsigned char   rom[PML] = {0x02, 0x00, 0x0F, 0x3C, 0x21, 0x0F, 0x3F, 0x22,
							0x30, 0x63, 0x62, 0x61, 0x31, 0x60, 0x01, 0x37,
							0x10, 0x6F, 0x10, 0x4F, 0xA3, 0x24, 0x11, 0xCA,
							0x21, 0x30, 0x21, 0x24, 0x27, 0x10, 0xDE, 0x10,
							0x6F, 0x44, 0x21, 0x42, 0x24, 0x26, 0x60, 0x45,
							0x21, 0x43, 0x26, 0x61, 0x24, 0x11, 0xC3, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

unsigned char   ram[DML] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	int 	lcnt = 0;
	int		addr = 2;

	int		icnt = 0;
	int		dcnt = 0;
	int     rcnt = 0;
	int		wcnt = 0;

	if(argc < (NUM_ARG + 1)) {
		printf("\n---Error: Number of arguments less than %d\n", NUM_ARG);
		printf("\tArgument List: lcnt = argv[1] -- loop count\n");
		printf("\t               addr = argv[2] -- end of loop address\n");
		return (-1);
	}

	lcnt = atoi(argv[1]);
	addr = atoi(argv[2]);

	// Initialize CPU state

    cpu.op = RST;
    cpu_model(&cpu);

    // Fetch and execute program continuously

    while(lcnt) {
        printf("ma: %04X;", cpu.ma);

        // Perform program memory operation

        if(IF == cpu.op) {
        	printf(" IF;");
        	cpu.io = rom[cpu.ma % PML];
        	icnt++; if(cpu.ma == addr) lcnt--;
        } else if(RD == cpu.op) {
        	printf(" RD;");
        	cpu.io = ram[cpu.ma % DML];
        	dcnt++; rcnt++;
        } else if(WR == cpu.op) {
        	printf(" WR;");
        	ram[cpu.ma % DML] = cpu.io;
        	dcnt++; wcnt++;
        }

        printf(" io: %02X;", cpu.io);

        // Call processor model

        cpu_model(&cpu);

        // Print out remainder of the processor state

        printf(" ir: %02X; ki: %04X;",
               cpu.ir,
               cpu.ki );

        printf(" ip: %04X; xp: %04X; yp: %04X; ys: %04X;",
        	   cpu.ip,
			   cpu.xp,
               cpu.yp,
               cpu.ys );

        printf(" ra: %02X; rb: %02X; cy: %d;",
               cpu.ra,
               cpu.rb,
			   cpu.cy );

        printf(" clr_ki: %d;", cpu.clr_ki);

//        ch = getchar();
        printf("\n");
    }

    printf("\n");
    lcnt = atoi(argv[1]);
    printf("Iterations            : %6d\n", lcnt);
    printf("Instruction Count     : %6d\n", icnt);
    printf("Data Access Count     : %6d\n", dcnt);
    printf("Read Access Count     : %6d\n", rcnt);
    printf("Write Access Count    : %6d\n", wcnt);
    printf("Clocks per Instruction: %6.3lf\n", (((float) (icnt + dcnt)) / icnt));

	return 0;
}
