-- IF/ID stage pipeline register

-- Sho Ko
-- 903197992
-- Add flush and stall signals, flush and stall reg1 accordingly.

Library IEEE;
use IEEE.std_logic_1164.all;
 
entity pipe_reg1 is
port (	if_PC4 : in std_logic_vector(31 downto 0);
	if_instruction: in std_logic_vector( 31 downto 0);
	clk, reset, stall, flush: in std_logic; -- add input signals
	id_PC4 : out std_logic_vector(31 downto 0);
	id_instruction: out std_logic_vector( 31 downto 0));
end pipe_reg1;

architecture behavioral of pipe_reg1 is
begin
process
begin
wait until (rising_edge(clk));
if reset = '1' or flush = '1' then -- when flush is needed
id_PC4 <= x"00000000";
id_instruction <= x"00000000";
elsif stall = '0' then -- when stall is not needed
id_PC4 <= if_PC4;
id_instruction <= if_instruction;
end if;
end process;
end behavioral;