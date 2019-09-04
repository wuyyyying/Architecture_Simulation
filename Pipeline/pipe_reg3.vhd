-- EX/MEM stage pipeline register

-- Sho Ko
-- 903197992
-- Add flush signal to flush reg3 as needed.

Library IEEE;
use IEEE.std_logic_1164.all;

entity pipe_reg3 is
port (ex_MemToReg, ex_RegWrite, ex_MemWrite, ex_MemRead, ex_branch, ex_zero: in std_logic;
      ex_alu_result, ex_register_rt, ex_branch_PC , ex_instruction, writeB: in std_logic_vector(31 downto 0); -- add input signals
      ex_wreg_addr :in std_logic_vector(4 downto 0);
      clk, reset, flush: in std_logic; -- add input signals

      mem_MemToReg, mem_RegWrite, mem_MemWrite, mem_MemRead, mem_branch, mem_zero: out std_logic;
      mem_alu_result, mem_register_rt, mem_branch_PC, mem_instruction : out std_logic_vector(31 downto 0); -- add output signals
      mem_wreg_addr :out std_logic_vector(4 downto 0));
end pipe_reg3;


architecture behavioral of pipe_reg3 is
begin
process
begin
wait until (rising_edge(clk));
if reset ='1' or flush = '1' then  -- when flush is needed
	mem_zero <= '0';
	mem_branch <= '0';
	mem_MemToReg <= '0';
	 mem_RegWrite <= '0';
	 mem_MemWrite <= '0';
	 mem_MemRead <= '0';
	 mem_alu_result <= x"00000000";
	 mem_register_rt  <= x"00000000";
	 mem_wreg_addr <= "00000";
	 mem_branch_PC <= x"00000000";
	mem_instruction <= x"00000000";              

else

	mem_branch_PC <=  ex_branch_PC;
	mem_zero <= ex_zero;
	mem_branch <= ex_branch;
	mem_MemToReg <= ex_MemToReg;
	 mem_RegWrite <= ex_RegWrite;
	 mem_MemWrite <= ex_MemWrite;
	 mem_MemRead <= ex_MemRead;
	 mem_alu_result <= ex_alu_result;
	 mem_register_rt  <= writeB; -- connect two signals
	 mem_wreg_addr <= ex_wreg_addr;
	mem_instruction <= ex_instruction; -- connect two signals
end if;

end process;
end behavioral;