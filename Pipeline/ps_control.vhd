-- control unit. simply implements the truth table for a small set of
-- instructions 

-- Sho Ko
-- 903197992
-- Add load-to-use and branch detection.

Library IEEE;
use IEEE.std_logic_1164.all;

entity control is
port(opcode, opcode_ex: in std_logic_vector(5 downto 0);
     ex_wreg_addr : in std_logic_vector(4 downto 0);
     instruction, instruction_ex, instruction_mem : in std_logic_vector(31 downto 0); -- add input to detect hazard
     mem_branch, mem_zero: in std_logic; -- add input to detect hazard

     RegDst, MemRead, MemToReg, MemWrite, stall, flush :out  std_logic; -- add output to send hazard handler
     ALUSrc, RegWrite, Branch: out std_logic;
     ALUOp: out std_logic_vector(1 downto 0));
end control;

architecture behavioral of control is

signal rformat, lw, sw, beq , hazard :std_logic; -- add hazard internal signal
begin 
--
-- recognize opcode for each instruction type
-- these variable should be inferred as wires	 

	rformat 	<=  '1'  WHEN  Opcode = "000000"  ELSE '0';
	Lw          <=  '1'  WHEN  Opcode = "100011"  ELSE '0';
 	Sw          <=  '1'  WHEN  Opcode = "101011"  ELSE '0';
   	Beq         <=  '1'  WHEN  Opcode = "000100"  ELSE '0';

--
-- implement each output signal as the column of the truth
-- table  which defines the control

-- implement stall logic which is when there is load-to-use and data hazard
stall <= '1' when (Opcode ="000000" and opcode_ex ="100011" and hazard ='1') else
 	 '0';

-- implement hazard logic which is when there is data hazard between two consecutive instructions
hazard <= '1' when (instruction(25 downto 21) = ex_wreg_addr or instruction(20 downto 16)= ex_wreg_addr) else
	  '0';

-- implement flush logic which is when rs is equal to rs and tis is branch instruction 
flush <= '1' when mem_branch = '1' and mem_zero = '1' else
         '0';

RegDst <= rformat;
ALUSrc <= (lw or sw) ;
MemToReg <= lw ;
RegWrite <= (rformat or lw);
MemRead <= lw ;
MemWrite <= sw;	   
Branch <= beq;

ALUOp(1 downto 0) <=  rformat & beq; -- note the use of the concatenation operator
				     -- to form  2 bit signal
end behavioral;