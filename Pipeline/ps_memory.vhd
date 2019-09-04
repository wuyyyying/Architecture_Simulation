-- ECE 3056: Architecture, Concurrency and Energy in Computation
-- Sudhakar Yalamanchili
-- Pipelined MIPS Processor VHDL Behavioral Mode--
----
-- data memory component.   

-- Sho Ko
-- 903197992
-- No changes made.

Library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;

entity  memory is  
port(
-- 
-- inputs
--
     address, write_data    : in std_logic_vector(31 downto 0);
     MemWrite, MemRead      : in std_logic;
--
-- outputs
--
     read_data :out std_logic_vector(31 downto 0));

end memory;


architecture behavioral of memory is 

TYPE DATA_RAM IS ARRAY (0 to 31) OF STD_LOGIC_VECTOR (31 DOWNTO 0);
   SIGNAL dram: DATA_RAM := (
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
	
BEGIN 				

-- memory read operation
read_data <= dram(CONV_INTEGER('0' & address(6 downto 2))) when MemRead = '1'
           else X"FFFFFFFF";         

-- memory write operation
dram(CONV_INTEGER('0' & address(6 downto 2))) <= write_data when MemWrite = '1' 
            else dram(CONV_INTEGER('0' & address(6 downto 2)));
			 	
end behavioral;