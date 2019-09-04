-- ECE 3056: Architecture, Concurrency and Energy in Computation
-- Sudhakar Yalamanchili
-- Pipelined MIPS Processor VHDL Behavioral Mode--
--
--
-- instruction decode unit.
--
-- Note that this module differs from the text in the following ways
-- 1. The MemToReg Mux is implemented in this module instead of a (syntactically) 
-- different pipeline stage.

-- Sho Ko
-- 903197992
-- Add output wreg_rs signal, which is the address of rs.

Library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
USE IEEE.STD_LOGIC_UNSIGNED.ALL;

entity decode is
port(
--
-- inputs 
--
     instruction : in std_logic_vector(31 downto 0);
     memory_data, alu_result :in std_logic_vector(31 downto 0);
     RegWrite, MemToReg, reset : in std_logic;
     wreg_address : in std_logic_vector(4 downto 0);
--
-- outputs
--
     register_rs, register_rt :out std_logic_vector(31 downto 0);
     Sign_extend :out std_logic_vector(31 downto 0);
     wreg_rd, wreg_rt, wreg_rs : out std_logic_vector (4 downto 0)); -- add wreg_rs signal
end decode;


architecture behavioral of decode is 
TYPE register_file IS ARRAY ( 0 TO 31 ) OF STD_LOGIC_VECTOR( 31 DOWNTO 0 );

	SIGNAL register_array: register_file := (
      X"00000000",
      X"11111111",
      X"22222222",
      X"33333333",
      X"44444444",
      X"55555555",
      X"66666666",
      X"77777777",
      X"0000000A",
      X"1111111A",
      X"2222222A",
      X"3333333A",
      X"4444444A",
      X"5555555A",
      X"6666666A",
      X"7777777A",
      X"0000000B",
      X"1111111B",
      X"2222222B",
      X"3333333B",
      X"4444444B",
      X"5555555B",
      X"6666666B",
      X"7777777B",
      X"000000BA",
      X"111111BA",
      X"222222BA",
      X"333333BA",
      X"444444BA",
      X"555555BA",
      X"666666BA",
      X"777777BA"
   );
	SIGNAL write_data					            : STD_LOGIC_VECTOR( 31 DOWNTO 0 );
	SIGNAL read_register_1_address		  : STD_LOGIC_VECTOR( 4 DOWNTO 0 );
	SIGNAL read_register_2_address		  : STD_LOGIC_VECTOR( 4 DOWNTO 0 );
	SIGNAL Instruction_immediate_value	: STD_LOGIC_VECTOR( 15 DOWNTO 0 );
	
	begin
        read_register_1_address 	<= Instruction( 25 DOWNTO 21 );
   	read_register_2_address 	<= Instruction( 20 DOWNTO 16 );
        Instruction_immediate_value 	<= Instruction( 15 DOWNTO 0 );
	
	write_data <= ALU_result( 31 DOWNTO 0 ) 
			           WHEN ( MemtoReg = '0' ) 	
			           ELSE memory_data;

	-- Sign Extend 16-bits to 32-bits
    	Sign_extend <= X"0000" & Instruction_immediate_value
		         WHEN Instruction_immediate_value(15) = '0'
		         ELSE	X"FFFF" & Instruction_immediate_value;
		
	-- Read Register 1 Operation		 
		register_rs <= register_array( 
			      CONV_INTEGER( read_register_1_address ) );
	-- Read Register 2 Operation		 
	   register_rt <= register_array( 
			      CONV_INTEGER( read_register_2_address ) );
	-- Register write operation
		
		register_array( CONV_INTEGER(wreg_address)) <= write_data
		                  when RegWrite = '1' else
		                      register_array(conv_integer(wreg_address));
	
	-- move possible write destinations to execute stage                 
	wreg_rd <= instruction(15 downto 11);
        wreg_rt <= instruction(20 downto 16);
	wreg_rs <= instruction(25 downto 21); -- implement wreg_rs
end behavioral;