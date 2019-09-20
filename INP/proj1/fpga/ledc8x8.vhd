--Alex Sporni, xsporn01@stud.fit.vutbr.cz, INP proj1, 29.9.2018--
library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;

entity ledc8x8 is
port ( -- Sem doplnte popis rozhrani obvodu.
	RESET				:	in std_logic;
	SMCLK				:	in std_logic;
	ROW				:	out std_logic_vector (0 to 7);
	LED				:	out std_logic_vector (0 to 7)
);
end ledc8x8;

architecture main of ledc8x8 is

    -- Sem doplnte definice vnitrnich signalu.
	signal pocitadlo : std_Logic_vector(11 downto 0) := (others => '0');
	signal change_state : std_Logic_vector(20 downto 0) := (others => '0');
	signal state : std_Logic_vector(1 downto 0) := "00";
	signal sig : std_logic;
	signal LED_VYSTUP : std_logic_vector (0 to 7) := (others => '0');
	signal ROW_VYSTUP :std_logic_vector (7 downto 0) := (others => '0');
begin

    -- Sem doplnte popis obvodu. Doporuceni: pouzivejte zakladni obvodove prvky
    -- (multiplexory, registry, dekodery,...), jejich funkce popisujte pomoci
    -- procesu VHDL a propojeni techto prvku, tj. komunikaci mezi procesy,
    -- realizujte pomoci vnitrnich signalu deklarovanych vyse.

    -- DODRZUJTE ZASADY PSANI SYNTETIZOVATELNEHO VHDL KODU OBVODOVYCH PRVKU,
    -- JEZ JSOU PROBIRANY ZEJMENA NA UVODNICH CVICENI INP A SHRNUTY NA WEBU:
    -- http://merlin.fit.vutbr.cz/FITkit/docs/navody/synth_templates.html.

    -- Nezapomente take doplnit mapovani signalu rozhrani na piny FPGA
    -- v souboru ledc8x8.ucf.
	 
	 hod_signal: process(SMCLK, RESET)
		begin
			if RESET = '1' then 
				pocitadlo <= (others => '0');
			elsif rising_edge(SMCLK) then
			if (pocitadlo = "11100001000") then
			sig <= '1';
			pocitadlo <= (others => '0');
			else 
			sig <= '0';
				pocitadlo <= pocitadlo + 1;
			end if;
			end if;
		end process hod_signal;
		
		hod_signal_modif: process(SMCLK, RESET)
		begin
			if RESET = '1' then 
				change_state <= (others => '0');
			elsif rising_edge(SMCLK) then
			if change_state = "111000010000000000000" then
				state <= state + 1;
				change_state <= (others => '0');
			else
				change_state <= change_state + 1;
			end if;
			end if;
		end process hod_signal_modif;
	 

		rotor: process(RESET, sig, SMCLK)
		begin
			if RESET = '1' then
			ROW_VYSTUP <= "10000000";
			elsif SMCLK'event and SMCLK = '1' and sig = '1' then
			ROW_VYSTUP <= ROW_VYSTUP(0) & ROW_VYSTUP(7 downto 1);
			end if;
		end process rotor;


		dekoder : process(ROW_VYSTUP)
		begin
		
		if state = "00" then
		case ROW_VYSTUP is
			-- Projektcia pismena A
			when "10000000" => LED_VYSTUP <= "11111111";
			when "01000000" => LED_VYSTUP <= "11000111";
			when "00100000" => LED_VYSTUP <= "10111011";
			when "00010000" => LED_VYSTUP <= "10111011";
			when "00001000" => LED_VYSTUP <= "10000011";
			when "00000100" => LED_VYSTUP <= "10111011";
			when "00000010" => LED_VYSTUP <= "10111011";
			when "00000001" => LED_VYSTUP <= "10111011";
			when others => LED_VYSTUP <= "11111111";
			end case;
		end if ;
		
			if state = "01" then
			case ROW_VYSTUP is
	      -- bliknutie
			when "10000000" => LED_VYSTUP <= "11111111";
			when "01000000" => LED_VYSTUP <= "11111111";
			when "00100000" => LED_VYSTUP <= "11111111";
			when "00010000" => LED_VYSTUP <= "11111111";
			when "00001000" => LED_VYSTUP <= "11111111";
			when "00000100" => LED_VYSTUP <= "11111111";
			when "00000010" => LED_VYSTUP <= "11111111";
			when "00000001" => LED_VYSTUP <= "11111111";
			when others => LED_VYSTUP <= "11111111";
			end case;
		end if;
		
			if state = "10" then
			case ROW_VYSTUP is
			-- Projekcia pismena S
			when "10000000" => LED_VYSTUP <= "11111111";
			when "01000000" => LED_VYSTUP <= "11000011";
			when "00100000" => LED_VYSTUP <= "10111111";
			when "00010000" => LED_VYSTUP <= "10111111";
			when "00001000" => LED_VYSTUP <= "11000111";
			when "00000100" => LED_VYSTUP <= "11111011";
			when "00000010" => LED_VYSTUP <= "11111011";
			when "00000001" => LED_VYSTUP <= "10000111";
			when others => LED_VYSTUP <= "11111111";
			end case;
		end if;

			if state = "11" then
			case ROW_VYSTUP is
			--bliknutie
			when "10000000" => LED_VYSTUP <= "11111111";
			when "01000000" => LED_VYSTUP <= "11111111";
			when "00100000" => LED_VYSTUP <= "11111111";
			when "00010000" => LED_VYSTUP <= "11111111";
			when "00001000" => LED_VYSTUP <= "11111111";
			when "00000100" => LED_VYSTUP <= "11111111";
			when "00000010" => LED_VYSTUP <= "11111111";
			when "00000001" => LED_VYSTUP <= "11111111";
			when others => LED_VYSTUP <= "11111111";
			end case;
		end if;
	end process dekoder;
	
	LED <= LED_VYSTUP;
	ROW <= ROW_VYSTUP;

end main;
