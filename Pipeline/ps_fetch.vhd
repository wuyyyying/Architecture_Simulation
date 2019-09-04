-- ECE 3056: Architecture, Concurrency and Energy in Computation
-- Pipelined MIPS Processor VHDL Behavioral Mode--
--
---- ECE 3056: Architecture, Concurrency and Energy in Computation
-- Sudhakar Yalamanchili
-- Pipelined MIPS Processor VHDL Behavioral Mode--
--
--
-- Instruction fetch behavioral model. Instruction memory is
-- provided within this model. IF increments the PC,  
-- and writes the appropriate output signals.

-- Sho Ko
-- 903197992
-- Add stall signal, stall PC.

Library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.Std_logic_arith.all;
USE IEEE.STD_LOGIC_UNSIGNED.ALL;

entity fetch is 

port(     instruction  : out std_logic_vector(31 downto 0);
	  PC_out       : out std_logic_vector (31 downto 0);
	  Branch_PC    : in std_logic_vector(31 downto 0);
	  clock, reset, PCSource, stall:  in std_logic);            -- add stall signal
end fetch;

architecture behavioral of fetch is 
TYPE INST_MEM IS ARRAY (0 to 10) of STD_LOGIC_VECTOR (31 DOWNTO 0);
   SIGNAL iram : INST_MEM := (
X"00008020", -- add $s0 $zero $zero
X"02008820", -- add $s1 $s0 $zero
X"1000FFFD", -- beq $zero $zero -2
X"01090020", -- add $zero $t0 $t1
X"012A8020", -- add $s0 $t1 $t2
X"8C10000C", -- lw $s0 12($zero)
X"00000000",
X"00000000",
X"00000000",
X"00000000",
X"00000000"
   );
   
SIGNAL PC, Next_PC : STD_LOGIC_VECTOR( 31 DOWNTO 0 );

BEGIN 						
-- access instruction pointed to by current PC
-- and increment PC by 4. This is combinational
		             
Instruction <=  iram(CONV_INTEGER(PC(4 downto 2)));  -- since the instruction
                                                     -- memory is indexed by integer
PC_out<= (PC + 4) when stall = '0' else -- when stall is not needed
  	 PC;			
   
-- compute value of next PC

Next_PC <=  (PC + 4)     when PCSource = '0' and stall = '0' else -- when stall is not needed
            Branch_PC    when PCSource = '1' and stall = '0' else -- when stall is not needed
	    PC           when stall ='1' else -- when stall is needed
            X"CCCCCCCC";
			   
-- update the PC on the next clock			   
	PROCESS
		BEGIN
			WAIT UNTIL (rising_edge(clock));
			IF (reset = '1') THEN
				PC<= X"00000000" ;
			ELSE 
				PC <= Next_PC;    -- cannot read/write a port hence need to duplicate info
			end if;
			 
	END PROCESS;   
end behavioral;