-- cpu.vhd: Simple 8-bit CPU (BrainF*ck interpreter)
-- Copyright (C) 2018 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Alex Sporni <xsporn01@stud.fit.vutbr.cz>
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity cpu is
 port (
   CLK   : in std_logic;  -- hodinovy signal
   RESET : in std_logic;  -- asynchronni reset procesoru
   EN    : in std_logic;  -- povoleni cinnosti procesoru
 
   -- synchronni pamet ROM
   CODE_ADDR : out std_logic_vector(11 downto 0); -- adresa do pameti
   CODE_DATA : in std_logic_vector(7 downto 0);   -- CODE_DATA <- rom[CODE_ADDR] pokud CODE_EN='1'
   CODE_EN   : out std_logic;                     -- povoleni cinnosti
   
   -- synchronni pamet RAM
   DATA_ADDR  : out std_logic_vector(9 downto 0); -- adresa do pameti
   DATA_WDATA : out std_logic_vector(7 downto 0); -- mem[DATA_ADDR] <- DATA_WDATA pokud DATA_EN='1'
   DATA_RDATA : in std_logic_vector(7 downto 0);  -- DATA_RDATA <- ram[DATA_ADDR] pokud DATA_EN='1'
   DATA_RDWR  : out std_logic;                    -- cteni z pameti (DATA_RDWR='1') / zapis do pameti (DATA_RDWR='0')
   DATA_EN    : out std_logic;                    -- povoleni cinnosti
   
   -- vstupni port
   IN_DATA   : in std_logic_vector(7 downto 0);   -- IN_DATA obsahuje stisknuty znak klavesnice pokud IN_VLD='1' a IN_REQ='1'
   IN_VLD    : in std_logic;                      -- data platna pokud IN_VLD='1'
   IN_REQ    : out std_logic;                     -- pozadavek na vstup dat z klavesnice
   
   -- vystupni port
   OUT_DATA : out  std_logic_vector(7 downto 0);  -- zapisovana data
   OUT_BUSY : in std_logic;                       -- pokud OUT_BUSY='1', LCD je zaneprazdnen, nelze zapisovat,  OUT_WE musi byt '0'
   OUT_WE   : out std_logic                       -- LCD <- OUT_DATA pokud OUT_WE='1' a OUT_BUSY='0'
 );
end cpu;


-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of cpu is

 -- zde dopiste potrebne deklarace signalu
    ------------------------------REGISTRE------------------------------
    ----------Programový_čítač(PC)----------
    signal reg_pc_data : std_logic_vector(11 downto 0):= (others => '0'); -- PC
    signal reg_pc_incr : std_logic; -- inkrementácia čítača
    signal reg_pc_decr : std_logic; -- dekremdekrementácia  čítača
    
    ----------Ukazatel_do_pamäte(PTR)----------
    signal reg_ptr_data : std_logic_vector(9 downto 0):= (others => '0'); -- prečíta hodnotu pamäte
    signal reg_ptr_incr : std_logic; -- (++)
    signal reg_ptr_decr : std_logic; -- (--)

    ----------Určenie_cyklov(CNT)----------
    signal reg_cnt_data : std_logic_vector(7 downto 0):= (others => '0'); -- vynulovanie
    signal reg_cnt_incr : std_logic; -- (++)
    signal reg_cnt_decr : std_logic; -- (--)
    

    ----------Multiplexor(MUX)----------
    signal mux_data_wdata : std_logic_vector(7 downto 0) := (others => '0');
    signal mux_data_wdata_sel : std_logic_vector(1 downto 0) := "00";


    ------------------------------AUTOMATA_STATES------------------------------
    type fsm_state is (
      FSM_init, 
      FSM_decode, 
      FSM_incr_ptr, FSM_decr_ptr,
      FSM_incr_cell, FSM_incr_cell2, FSM_decr_cell, FSM_decr_cell2,
      FSM_while_begin, FSM_while_begin2, FSM_while_begin3, FSM_while_begin4, 
      FSM_while_end, FSM_while_end2, FSM_while_end3, FSM_while_end4, FSM_while_end5, FSM_while_end6,
      FSM_print_actuall_cell, FSM_print_actuall_cell2, FSM_getchar_cell,
      FSM_block_commentary, FSM_block_commentary2, FSM_block_commentary3,
      FSM_cell_overwrite_hexa,
      FSM_zero_nine_a_f, FSM_return, FSM_other_state
      
    );
    ------------------------------AUTOMATA_SIGNALS------------------------------
    signal FSM_next_state : fsm_state;
    signal FSM_present_state : fsm_state;
begin

 -- zde dopiste vlastni VHDL kod dle blokoveho schema
 -- inspirujte se kodem procesoru ze cviceni

----------REGISTER(PC)----------
reg_pc: process(CLK, RESET, reg_pc_incr, reg_pc_decr, reg_pc_data)
  begin
    if (RESET = '1') then
      reg_pc_data <= (others => '0');
    elsif (CLK'event and CLK = '1') then
      if (reg_pc_incr = '1') then
        reg_pc_data <= reg_pc_data + 1;
    elsif (reg_pc_decr = '1') then
        reg_pc_data <= reg_pc_data - 1;
        end if;
    end if;
  end process;

  CODE_ADDR <= reg_pc_data;  

----------REGISTER(PTR)----------  
reg_ptr: process(CLK, RESET, reg_ptr_incr, reg_ptr_decr, reg_ptr_data)
  begin
    if (RESET = '1') then
      reg_ptr_data <= (others => '0');
    elsif (CLK'event and CLK = '1') then
      if (reg_ptr_incr = '1') then
        reg_ptr_data <= reg_ptr_data + 1;
    elsif (reg_ptr_decr = '1') then
        reg_ptr_data <= reg_ptr_data - 1;
        end if;
    end if;
  end process;

  DATA_ADDR <= reg_ptr_data;
  

----------REGISTER(CNT)----------  
reg_cnt: process(CLK, RESET, reg_cnt_incr, reg_cnt_decr)
  begin
    if (RESET = '1') then
      reg_cnt_data <= (others => '0');
    elsif (CLK'event and CLK = '1') then
      if (reg_cnt_incr = '1') then
        reg_cnt_data <= reg_cnt_data + 1;
    elsif (reg_cnt_decr = '1') then
        reg_cnt_data <= reg_cnt_data - 1;
        end if;
    end if;
  end process;
  
----------MULTIPLEXOR(MUX)---------- 
mux: process(IN_DATA, DATA_RDATA, mux_data_wdata, mux_data_wdata_sel)
  begin
    case(mux_data_wdata_sel) is
      when "00" => DATA_WDATA <= IN_DATA;
      when "01" => DATA_WDATA <= DATA_RDATA + 1;
      when "10" => DATA_WDATA <= DATA_RDATA - 1;
      when "11" => DATA_WDATA <= mux_data_wdata;
      when others =>
    end case;
  end process;

----------FSM(present_state)---------- 
present_state : process(CLK, RESET)
      begin
        if (RESET = '1') then
          FSM_present_state <= FSM_init;
          elsif (CLK'event and CLK = '1') then
            if (EN = '1') then
              FSM_present_state <= FSM_next_state;
            end if;
        end if;
      end process;
      
----------FSM(next_state)---------- 
next_state : process(DATA_RDATA, CODE_DATA, IN_VLD, OUT_BUSY, FSM_present_state, reg_cnt_data)            
              begin
                --inicialization section--
                --inicialization of cpu, regs and mux--
                --cpu--
                CODE_EN <= '1';
                DATA_RDWR <= '0';
                DATA_EN <= '0';
                IN_REQ <= '0';
                OUT_WE <= '0';
               
                --regs--
                reg_pc_incr <= '0';
                reg_pc_decr <= '0';
                reg_cnt_incr <= '0';
                reg_cnt_decr <= '0';
                reg_ptr_incr <= '0';
                reg_ptr_decr <= '0';
                
                --mux--
                mux_data_wdata_sel <= "00";
                
                --FSM_cell_overwrite_hexa <= "0";
                --automata--
                case (FSM_present_state) is
                  when FSM_init => CODE_EN <= '1';
                       FSM_next_state <= FSM_decode;

                  when FSM_decode =>
                    case (CODE_DATA) is
                      when X"3E" => FSM_next_state <= FSM_incr_ptr; -- '>' --
                      when X"3C" => FSM_next_state <= FSM_decr_ptr; -- '<' --
                      when X"2B" => FSM_next_state <= FSM_incr_cell; -- '+' --
                      when X"2D" => FSM_next_state <= FSM_decr_cell; -- '-' --
                      when X"5B" => FSM_next_state <= FSM_while_begin; -- '[' --
                      when X"5D" => FSM_next_state <= FSM_while_end; -- ']' --
                      when X"2E" => FSM_next_state <= FSM_print_actuall_cell; -- '.' --
                      when X"2C" => FSM_next_state <= FSM_getchar_cell; -- ',' --
                      when X"23" => FSM_next_state <= FSM_block_commentary; -- '#' --
                      when X"30" => FSM_next_state <= FSM_cell_overwrite_hexa; -- '0' --
                      when X"31" => FSM_next_state <= FSM_cell_overwrite_hexa; -- '1' --
                      when X"32" => FSM_next_state <= FSM_cell_overwrite_hexa; -- '2' --
                      when X"33" => FSM_next_state <= FSM_cell_overwrite_hexa; -- '3' --
                      when X"34" => FSM_next_state <= FSM_cell_overwrite_hexa; -- '4' --
                      when X"35" => FSM_next_state <= FSM_cell_overwrite_hexa; -- '5' --
                      when X"36" => FSM_next_state <= FSM_cell_overwrite_hexa; -- '6' --
                      when X"37" => FSM_next_state <= FSM_cell_overwrite_hexa; -- '7' --
                      when X"38" => FSM_next_state <= FSM_cell_overwrite_hexa; -- '8' --
                      when X"39" => FSM_next_state <= FSM_cell_overwrite_hexa; -- '9' --
                      when X"41" => FSM_next_state <= FSM_zero_nine_a_f; -- 'A' --
                      when X"42" => FSM_next_state <= FSM_zero_nine_a_f; -- 'B' --
                      when X"43" => FSM_next_state <= FSM_zero_nine_a_f; -- 'C' --
                      when X"44" => FSM_next_state <= FSM_zero_nine_a_f; -- 'D' --
                      when X"45" => FSM_next_state <= FSM_zero_nine_a_f; -- 'E' --
                      when X"46" => FSM_next_state <= FSM_zero_nine_a_f; -- 'F' --
                      when X"00" => FSM_next_state <= FSM_return; -- 'return' --
                      when others => FSM_next_state <= FSM_other_state; -- others ? --
                    end case;
                  
                    -- '>' --
                  when FSM_incr_ptr =>
                      reg_ptr_incr <= '1';
                      reg_pc_incr <= '1';
                      FSM_next_state <= FSM_init;

                  -- '<' -- 
                  when  FSM_decr_ptr =>
                      reg_ptr_decr <= '1';
                      reg_pc_incr <= '1';
                      FSM_next_state <= FSM_init;

                  -- '+' --
                  when FSM_incr_cell =>
                      DATA_EN <= '1';
                      DATA_RDWR <= '1';
                      FSM_next_state <= FSM_incr_cell2;

                  when FSM_incr_cell2 =>
                      mux_data_wdata_sel <= "01";
                      DATA_EN <= '1';
                      DATA_RDWR <= '0';
                      reg_pc_incr <= '1';
                      FSM_next_state <= FSM_init;

                  -- '-' --
                  when FSM_decr_cell =>
                      DATA_EN <= '1';
                      DATA_RDWR <= '1';
                      FSM_next_state <= FSM_decr_cell2;

                  when FSM_decr_cell2 =>
                      mux_data_wdata_sel <= "10";
                      DATA_EN <= '1';
                      DATA_RDWR <= '0';
                      reg_pc_incr <= '1';
                      FSM_next_state <= FSM_init;

                  -- '.' --
                  when FSM_print_actuall_cell =>
                      if (OUT_BUSY = '1') then
                        FSM_next_state <= FSM_print_actuall_cell;
                      else
                        DATA_EN <= '1';
                        DATA_RDWR <= '1';
                        FSM_next_state <= FSM_print_actuall_cell2;
                      end if;

                  when FSM_print_actuall_cell2 =>
                      OUT_WE <= '1';
                      OUT_DATA <= DATA_RDATA;
                      reg_pc_incr <= '1';
                      FSM_next_state <= FSM_init;

                  -- ',' --
                  when FSM_getchar_cell =>
                      IN_REQ <= '1';
                      if (IN_VLD = '0') then
                        FSM_next_state <= FSM_getchar_cell;
                      else
                          mux_data_wdata_sel <= "00";
                          DATA_EN <= '1';
                          DATA_RDWR <= '0';
                          reg_pc_incr <= '1';
                          FSM_next_state <= FSM_init;
                      end if;

                  -- '[' --
                  when FSM_while_begin =>
                      reg_pc_incr <= '1';
                      DATA_EN <= '1';
                      DATA_RDWR <= '1';
                      FSM_next_state <= FSM_while_begin2;

                  when FSM_while_begin2 =>
                      if (DATA_RDATA = X"00") then
                        reg_cnt_incr <= '1';
                        FSM_next_state <= FSM_while_begin3;
                      else
                        FSM_next_state <= FSM_init;
                      end if;

                  when FSM_while_begin3 =>
                      if (reg_cnt_data = X"00") then
                        FSM_next_state <= FSM_init;
                      else
                        CODE_EN <= '1';
                        FSM_next_state <= FSM_while_begin4;
                      end if;

                  when FSM_while_begin4 =>
                      if (CODE_DATA = X"5B") then
                        reg_cnt_incr <= '1';
                      elsif (CODE_DATA = X"5D") then
                        reg_cnt_decr <= '1';
                      end if;
                      reg_pc_incr <= '1';
                      FSM_next_state <= FSM_while_begin3;

                  -- ']' --
                  when FSM_while_end =>
                      DATA_EN <= '1';
                      DATA_RDWR <= '1';
                      FSM_next_state <= FSM_while_end2;

                  when FSM_while_end2 =>
                      if(DATA_RDATA = X"00") then
                        reg_pc_incr <= '1';
                        FSM_next_state <= FSM_init;
                      else
                        FSM_next_state <= FSM_while_end3;
                      end if;

                  when FSM_while_end3 =>
                      reg_cnt_incr <= '1';
                      reg_pc_decr <= '1';
                      FSM_next_state <= FSM_while_end4;

                  when FSM_while_end4 =>
                      if(reg_cnt_data = "00000000") then
                        FSM_next_state <= FSM_init;
                      else
                          CODE_EN <= '1';
                          FSM_next_state <= FSM_while_end5;
                      end if;

                  when FSM_while_end5 =>
                      if (CODE_DATA = X"5D") then
                          reg_cnt_incr <= '1';
                      elsif(CODE_DATA = X"5B") then
                          reg_cnt_decr <= '1';
                      end if;
                      FSM_next_state <= FSM_while_end6;

                  when FSM_while_end6 =>
                      if(reg_cnt_data = "00000000") then
                        reg_pc_incr <= '1';
                      else
                        reg_pc_decr <= '1';
                      end if;
                      FSM_next_state <= FSM_while_end4;

                  -- '#' --
                  when FSM_block_commentary =>
                    reg_pc_incr <= '1';
                    FSM_next_state <= FSM_block_commentary2;

                  when FSM_block_commentary2 =>
                  CODE_EN <= '1';
                    FSM_next_state <= FSM_block_commentary3;

                  when FSM_block_commentary3 =>
                    if (CODE_DATA = X"23") then
                      reg_pc_incr <= '1';
                      FSM_next_state <= FSM_init;
                    else
                      FSM_next_state <= FSM_block_commentary;
                    end if;

                  -- '0-9'--
                  when FSM_cell_overwrite_hexa =>
                    DATA_EN <= '1';
                    reg_pc_incr <= '1';
                    mux_data_wdata_sel <= "11";
                    mux_data_wdata <= CODE_DATA(3 downto 0) & "0000";
                    FSM_next_state <= FSM_init;

                  -- 'A-F' --
                  when FSM_zero_nine_a_f =>
                    DATA_EN <= '1';
                    reg_pc_incr <= '1';
                    mux_data_wdata_sel <= "11";
                    mux_data_wdata <= (CODE_DATA(3 downto 0) + std_logic_vector(conv_unsigned(9, mux_data_wdata'LENGTH)(3 downto 0))) & X"0";
                    FSM_next_state <= FSM_init;

                  -- 'return' --
                  when FSM_return =>
                    FSM_next_state <= FSM_return;

                    ---- others ? ---
                  when FSM_other_state =>
                    reg_pc_incr <= '1';
                    FSM_next_state <= FSM_init;
                  when others =>
                end case;
  end process;

end behavioral;