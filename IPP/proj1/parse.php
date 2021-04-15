<?php
/**
 * IPP project: parse.php
 * Author: Alex Sporni
 * Email: xsporn01@stud.fit.vutbr.cz
 * Date: 10.3.2020
 */

##### <----------> REGEX DEFINITION <----------> #####
/**
 * generated on https://regex101.com/
*/
define("REGEX_VAR","/^(?:GF|LF|TF)@[a-zA-Z\-_$&%*!?][a-zA-Z0-9\-_$&%*!?]*$/");
define("REGEX_STRING","/^string@(?:\\\\\d{3}|[^\\\\\s#])*$/");
define("REGEX_INT","/^(?:int@[-+]?\d+)$/");
define("REGEX_BOOL","/^bool@(?:true|false)$/");
define("REGEX_NIL","/^(?:nil)[@](?:nil)$/");
define("REGEX_TYPE","/^(?:int|string|bool)$/");
define("REGEX_LABEL","/^[a-zA-Z\-_$&%*!?][a-zA-Z0-9\-_$&%*!?]*$/");
define("REGEX_COMMENT","/(?:#.*)/");

define("REG_VAR","/^(?:GF|LF|TF)@[a-zA-Z\-_$&%*!?][a-zA-Z0-9\-_$&%*!?]*$/");
define("REG_LABEL","/^[a-zA-Z\-_$&%*!?][a-zA-Z0-9\-_$&%*!?]*$/");
define("REG_SYMB","/^(?:(?:(?:GF|LF|TF)@[a-zA-Z\-_$&%*!?][a-zA-Z0-9\-_$&%*!?]*)|(?:string@(?:\\\\\d{3}|[^\\\\\s#])*)|(?:int@[-+]?\d+)|(?:bool@(?:true|false))|(?:nil@nil))$/");

##### <----------> STATISTICS COUNTER <----------> #####
$stats_counter = array("loc" => 0,"comments" => 0,"labels" => 0,"jumps" => 0);

/**
 * @brief Function provides basic help menu
*/
function get_help()
{	echo "*****HELP MENU*****\n";
	echo "  php7.4 parse.php [--help]\n";
	echo "  --help | -h --> Shows the help menu\n";
	echo "  Script of type filter (parse.php in language PHP 7.4\n";
	echo "  Script loads the source code IPPcode20\n";
	echo "  Script provides lexical and syntactic analysis\n";
	echo "  If the code is correct, the script prints the correct code in XML format on standard output\n";
	echo "  --stats=FILE	writes statistics into a specific FILE\n";
	echo " 		 --loc			number of lines with valid instructions\n";
	echo " 		 --jumps		number of jump instructions\n";
	echo " 		 --comments		number of lines with comments\n";
	echo " 		 --labels		number of defined labels\n";
	exit(0);
}
/**
 * @brief Function initializes XML variable and appends the default header
 */
function xml_code_init()
{
    global $xw;
    $xw = xmlwriter_open_memory();
    xmlwriter_set_indent($xw, 1);
    xmlwriter_set_indent_string($xw, '  ');
    xmlwriter_start_document($xw, '1.0', 'UTF-8');
    xmlwriter_start_element($xw, 'program');
    xmlwriter_start_attribute($xw, 'language');
    xmlwriter_text($xw, 'IPPcode20');
    xmlwriter_end_attribute($xw);
}

/**
 * @brief Function writes statistics to file
 * @param $options program options
 * @param $write file pointer
 * @param $stats array of stats which will be used to write to file
 */
function write_to_file($options, $write, $stats)
{
	foreach($options as $option => $value) 
	{
		switch($option) 
		{
            case "stats":
				break;
				
            case "loc":
				if (fwrite($write, $stats["loc"]."\n") == 0) 
				{
                    fwrite(STDERR, "Error, write operation has failed!\n");
                    exit(99);
                }
                break;
            case "comments":
				if(fwrite($write, $stats["comments"]."\n") == 0) 
				{
                    fwrite(STDERR, "Error, write operation has failed!\n");
                    exit(99);
                }
                break;
            case "labels":
				if(fwrite($write, $stats["labels"]."\n") == 0) 
				{
                    fwrite(STDERR, "Error, write operation has failed!\n");
                    exit(99);
                }
                break;
            case "jumps":
				if(fwrite($write, $stats["jumps"]."\n") == 0) 
				{
                    fwrite(STDERR, "Error, write operation has failed!\n");
                    exit(99);
                }
                break;
            default:
                fwrite(STDERR, "Error, write operation has failed!\n");
                exit(99);
        }
    }
}


/**
 * @brief Function provides correct XML type checking and assignment
 * @param $Arg Argument type
 */
function xml_type($Arg)
{	
	if (preg_match(REGEX_VAR, $Arg))
	{
		return ["var", $Arg];
	}
	if (preg_match(REGEX_STRING, $Arg))
	{
		$string_parsed = preg_replace("/\b(string@)/","", $Arg);
		return ["string", $string_parsed];
	}
	else if (preg_match(REGEX_INT , $Arg))
	{
		$string_parsed = preg_replace("/\b(int@)/","", $Arg);
		return ["int", $string_parsed];
	}
	else if (preg_match(REGEX_BOOL, $Arg))
	{
		$string_parsed = preg_replace("/\b(bool@)/","", $Arg);
		return ["bool", $string_parsed];
	}
	else if (preg_match(REGEX_NIL, $Arg))
	{
		$string_parsed = preg_replace("/\b(nil@)/","", $Arg);
		return ["nil", $string_parsed];
	}
	else if (preg_match(REGEX_TYPE, $Arg))
	{
		return ["type", $Arg];
	}
	else if (preg_match(REGEX_LABEL, $Arg))
	{
		return ["label", $Arg];
	}
	else
	{
		fwrite(STDERR, "Error, undefined regex pattern\n"); 
		exit(23);
	}
}
/**
 * @brief Function for writing XML output with zero parameters
 * @param $ins_order instruction order 
 * @param $instruction instruction type
 * @param $xw XML writer 
 */
function xmlwrite_zero_param($ins_order, $instruction, $xw) 
{
	xmlwriter_start_element($xw, 'instruction');
	xmlwriter_start_attribute($xw, 'order');
	xmlwriter_text($xw, $ins_order);
	xmlwriter_start_attribute($xw, 'opcode');
	xmlwriter_text($xw, strtoupper($instruction));
	xmlwriter_end_attribute($xw);
}
/**
 * @brief Function for writing XML output with one parameter
 * @param $ins_order instruction order 
 * @param $instruction instruction type
 * @param $xw XML writer 
 * @param $firstArg first argument 
 */
function xmlwrite_one_param($ins_order, $instruction, $xw, $firstArg)
{
	$type_first_param = xml_type($firstArg);

	xmlwriter_start_element($xw, 'instruction');
	xmlwriter_start_attribute($xw, 'order');
	xmlwriter_text($xw, $ins_order);
	xmlwriter_start_attribute($xw, 'opcode');
	xmlwriter_text($xw, strtoupper($instruction));
	xmlwriter_end_attribute($xw);

	xmlwriter_start_element($xw, 'arg1');
	xmlwriter_start_attribute($xw, 'type');
	xmlwriter_text($xw, $type_first_param[0]);
	xmlwriter_end_attribute($xw);
	xmlwriter_text($xw, $type_first_param[1]);
	xmlwriter_end_element($xw);
}
/**
 * @brief  Function for writing XML output with two parameters
 * @param $ins_order instruction order 
 * @param $instruction instruction type
 * @param $xw XML writer 
 * @param $firstArg first argument 
 * @param $secondArg second argument 
 */
function xmlwrite_two_param($ins_order, $instruction, $xw, $firstArg, $secondArg)
{
	$type_first_param = xml_type($firstArg);
	$type_second_param = xml_type($secondArg);

	xmlwriter_start_element($xw, 'instruction');
	xmlwriter_start_attribute($xw, 'order');
	xmlwriter_text($xw, $ins_order);
	xmlwriter_start_attribute($xw, 'opcode');
	xmlwriter_text($xw, strtoupper($instruction));
	xmlwriter_end_attribute($xw);

	xmlwriter_start_element($xw, 'arg1');
	xmlwriter_start_attribute($xw, 'type');
	xmlwriter_text($xw, $type_first_param[0]);
	xmlwriter_end_attribute($xw);
	xmlwriter_text($xw, $type_first_param[1]);
	xmlwriter_end_element($xw);

	xmlwriter_start_element($xw, 'arg2');
	xmlwriter_start_attribute($xw, 'type');
	xmlwriter_text($xw, $type_second_param[0]);
	xmlwriter_end_attribute($xw);
	xmlwriter_text($xw, $type_second_param[1]);
	xmlwriter_end_element($xw);
}
/**
 * @brief  Function for writing XML output with three parameters
 * @param $ins_order instruction order 
 * @param $instruction instruction type
 * @param $xw XML writer 
 * @param $firstArg first argument 
 * @param $secondArg second argument 
 * @param $thirdArg third argument
 */
function xmlwrite_three_param($ins_order, $instruction, $xw, $firstArg, $secondArg, $thirdArg)
{
	$type_first_param = xml_type($firstArg);
	$type_second_param = xml_type($secondArg);
	$type_third_param = xml_type($thirdArg);

	xmlwriter_start_element($xw, 'instruction');
	xmlwriter_start_attribute($xw, 'order');
	xmlwriter_text($xw, $ins_order);
	xmlwriter_start_attribute($xw, 'opcode');
	xmlwriter_text($xw, strtoupper($instruction));
	xmlwriter_end_attribute($xw);

	xmlwriter_start_element($xw, 'arg1');
	xmlwriter_start_attribute($xw, 'type');
	xmlwriter_text($xw, $type_first_param[0]);
	xmlwriter_end_attribute($xw);
	xmlwriter_text($xw, $type_first_param[1]);
	xmlwriter_end_element($xw);

	xmlwriter_start_element($xw, 'arg2');
	xmlwriter_start_attribute($xw, 'type');
	xmlwriter_text($xw, $type_second_param[0]);
	xmlwriter_end_attribute($xw);
	xmlwriter_text($xw, $type_second_param[1]);
	xmlwriter_end_element($xw);

	xmlwriter_start_element($xw, 'arg3');
	xmlwriter_start_attribute($xw, 'type');
	xmlwriter_text($xw, $type_third_param[0]);
	xmlwriter_end_attribute($xw);
	xmlwriter_text($xw,  $type_third_param[1]);
	xmlwriter_end_element($xw);
}

##### <----------> INPUT ARGUMENT PARSING <----------> #####
$longopts  = array("help", "stats:", "loc", "comments", "labels", "jumps");
$shortopts = "h";
$optind = null;
$options = getopt($shortopts, $longopts, $optind);
$number_of_options = count($options);
if (array_key_exists('h', $options) || array_key_exists('help', $options))
{
  if ($argc == 2)
  {
      get_help();
  }
  else
  {
    fwrite(STDERR, "Error, wrong number of arguments!\n");
    exit(10);
  }  
}
else if (array_key_exists("stats", $options))
{
    if (($number_of_options != ($argc-1)) || ($number_of_options <= 1))
    {
        fwrite(STDERR, "Error, invalid usage !\n");
		exit(10);
    }
    
}
else if ($argc > 1)
{
    fwrite(STDERR,"Error, invalid usage or wrong number of characters!\n");
    exit(10);
}

##### <----------> HEADER CHECKING (.IPPcode20) <----------> #####
$line = fgets(STDIN); // Takes only the first line from the STDIN
if (preg_match(REGEX_COMMENT, $line))
{
    $comment = preg_replace(REGEX_COMMENT,"", $line);
    $line = $comment;
}
$substring = explode(" ", $line);
$ippHeader = strtolower($substring[0]);
$ippHeader = trim(preg_replace('/\s\s+/', ' ', $ippHeader));
$ins_order = 1;
if (strcmp($ippHeader, ".ippcode20"))
{
	fwrite(STDERR, "Error, wrong header format!\n"); 
	exit(21);
}
xml_code_init();
while ($line = fgets(STDIN))
{
    if(preg_match(REGEX_COMMENT, $line)) // Comment handling
    {
        $comment = preg_replace(REGEX_COMMENT,"", $line);
		$line = $comment;
		$stats_counter["comments"]++;
    }
    $substring = explode(" ", $line);
	$instruction = strtoupper($substring[0]);
	$substring[1] = trim(preg_replace('/\s\s+/', ' ', $substring[1]));
	$substring[2] = trim(preg_replace('/\s\s+/', ' ', $substring[2]));
	$substring[3] = trim(preg_replace('/\s\s+/', ' ', $substring[3]));
    $firstArg = $substring[1];
	$secondArg = $substring[2];
    $thirdArg = $substring[3];
    $instruction = trim(preg_replace('/\s\s+/', ' ', $instruction)); 
    if (!(strcmp($instruction, "")))
	{
		continue;
    }
    switch ($instruction) 
	{
		##### Instructions without parameters ###############
		case "CREATEFRAME":
			if ($firstArg != NULL || $secondArg != NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			xmlwrite_zero_param($ins_order, $instruction, $xw);
			break;

		case "PUSHFRAME":
			if ($firstArg != NULL || $secondArg != NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			xmlwrite_zero_param($ins_order, $instruction, $xw);
			break;
		
		case "POPFRAME":
			if ($firstArg != NULL || $secondArg != NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			xmlwrite_zero_param($ins_order, $instruction, $xw);
			break;

		case "RETURN":
			if ($firstArg != NULL || $secondArg != NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			xmlwrite_zero_param($ins_order, $instruction, $xw);
			break;

		case "BREAK":
			if ($firstArg != NULL || $secondArg != NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			xmlwrite_zero_param($ins_order, $instruction, $xw);
			break;	

		##### Instructions with one parameter ###########################
		case "DEFVAR":
			if ($firstArg == NULL || $secondArg != NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_one_param($ins_order, $instruction, $xw, $firstArg);
			break;

		case "CALL":
			if ($firstArg == NULL || $secondArg != NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_LABEL, $firstArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_one_param($ins_order, $instruction, $xw, $firstArg);
			break;	

		case "PUSHS":
			if ($firstArg == NULL || $secondArg != NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_SYMB, $firstArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_one_param($ins_order, $instruction, $xw, $firstArg);
			break;

		case "POPS":
			if ($firstArg == NULL || $secondArg != NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_one_param($ins_order, $instruction, $xw, $firstArg);
			break;

		case "WRITE":
			if ($firstArg == NULL || $secondArg != NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_SYMB, $firstArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_one_param($ins_order, $instruction, $xw, $firstArg);
			break;

		case "LABEL":
			if ($firstArg == NULL || $secondArg != NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_LABEL, $firstArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_one_param($ins_order, $instruction, $xw, $firstArg);
			$stats_counter["labels"]++;
			break;

		case "JUMP":
			if ($firstArg == NULL || $secondArg != NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_LABEL, $firstArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_one_param($ins_order, $instruction, $xw, $firstArg);
            $stats_counter["jumps"]++;
			break;				

		case "EXIT":
			if ($firstArg == NULL || $secondArg != NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_SYMB, $firstArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_one_param($ins_order, $instruction, $xw, $firstArg);
			break;		

		case "DPRINT":
			if ($firstArg == NULL || $secondArg != NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_SYMB, $firstArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_one_param($ins_order, $instruction, $xw, $firstArg);
			break;		

		##### Instructions with two parameters #######################################
		case "MOVE":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg) && preg_match(REG_SYMB, $secondArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_two_param($ins_order, $instruction, $xw, $firstArg, $secondArg);
			break;	
		
		case "INT2CHAR":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg) && preg_match(REG_SYMB, $secondArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_two_param($ins_order, $instruction, $xw, $firstArg, $secondArg);
			break;	

		case "READ":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg) && preg_match(REG_TYPE, $secondArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_two_param($ins_order, $instruction, $xw, $firstArg, $secondArg);
			break;

		case "STRLEN":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg) && preg_match(REG_SYMB, $secondArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_two_param($ins_order, $instruction, $xw, $firstArg, $secondArg);
			break;	

		case "TYPE":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg) && preg_match(REG_SYMB, $secondArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_two_param($ins_order, $instruction, $xw, $firstArg, $secondArg);
            break;	
            
        case "NOT":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg != NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg) && preg_match(REG_SYMB, $secondArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_two_param($ins_order, $instruction, $xw, $firstArg, $secondArg);
            break;
	
		##### Instructions with three parameters ##################################################
		case "ADD":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg == NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg) && preg_match(REG_SYMB, $secondArg) && preg_match(REG_SYMB, $thirdArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_three_param($ins_order, $instruction, $xw, $firstArg, $secondArg, $thirdArg);
			break;	

		case "SUB":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg == NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg) && preg_match(REG_SYMB, $secondArg) && preg_match(REG_SYMB, $thirdArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_three_param($ins_order, $instruction, $xw, $firstArg, $secondArg, $thirdArg);
			break;

		case "MUL":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg == NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg) && preg_match(REG_SYMB, $secondArg) && preg_match(REG_SYMB, $thirdArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_three_param($ins_order, $instruction, $xw, $firstArg, $secondArg, $thirdArg);
			break;

		case "IDIV":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg == NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg) && preg_match(REG_SYMB, $secondArg) && preg_match(REG_SYMB, $thirdArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_three_param($ins_order, $instruction, $xw, $firstArg, $secondArg, $thirdArg);
			break;	

		case "LT":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg == NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg) && preg_match(REG_SYMB, $secondArg) && preg_match(REG_SYMB, $thirdArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_three_param($ins_order, $instruction, $xw, $firstArg, $secondArg, $thirdArg);
			break;

		case "GT":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg == NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg) && preg_match(REG_SYMB, $secondArg) && preg_match(REG_SYMB, $thirdArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_three_param($ins_order, $instruction, $xw, $firstArg, $secondArg, $thirdArg);
			break;

		case "EQ":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg == NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg) && preg_match(REG_SYMB, $secondArg) && preg_match(REG_SYMB, $thirdArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_three_param($ins_order, $instruction, $xw, $firstArg, $secondArg, $thirdArg);
			break;	

		case "AND":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg == NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg) && preg_match(REG_SYMB, $secondArg) && preg_match(REG_SYMB, $thirdArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_three_param($ins_order, $instruction, $xw, $firstArg, $secondArg, $thirdArg);
			break;

		case "OR":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg == NULL)
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg) && preg_match(REG_SYMB, $secondArg) && preg_match(REG_SYMB, $thirdArg)))
			{
				fwrite(STDERR, "Error, wrong instruction type\n");
				exit(23);
			}
			xmlwrite_three_param($ins_order, $instruction, $xw, $firstArg, $secondArg, $thirdArg);
			break;		
		
		case "STRI2INT":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg == NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg) && preg_match(REG_SYMB, $secondArg) && preg_match(REG_SYMB, $thirdArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_three_param($ins_order, $instruction, $xw, $firstArg, $secondArg, $thirdArg);
			break;

		case "CONCAT":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg == NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg) && preg_match(REG_SYMB, $secondArg) && preg_match(REG_SYMB, $thirdArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_three_param($ins_order, $instruction, $xw, $firstArg, $secondArg, $thirdArg);
			break;

		case "GETCHAR":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg == NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg) && preg_match(REG_SYMB, $secondArg) && preg_match(REG_SYMB, $thirdArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_three_param($ins_order, $instruction, $xw, $firstArg, $secondArg, $thirdArg);
			break;

		case "SETCHAR":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg == NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_VAR, $firstArg) && preg_match(REG_SYMB, $secondArg) && preg_match(REG_SYMB, $thirdArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_three_param($ins_order, $instruction, $xw, $firstArg, $secondArg, $thirdArg);
			break;

		case "JUMPIFEQ":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg == NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_LABEL, $firstArg) && preg_match(REG_SYMB, $secondArg) && preg_match(REG_SYMB, $thirdArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_three_param($ins_order, $instruction, $xw, $firstArg, $secondArg, $thirdArg);
            $stats_counter["jumps"]++;
			break;

		case "JUMPIFNEQ":
			if ($firstArg == NULL || $secondArg == NULL || $thirdArg == NULL)
			{
				fwrite(STDERR, "Error, wrong number of instruction parameters\n");
				exit(23);
			}
			if (!(preg_match(REG_LABEL, $firstArg) && preg_match(REG_SYMB, $secondArg) && preg_match(REG_SYMB, $thirdArg)))
			{
				fwrite(STDERR, "Error, wrong operand type\n");
				exit(23);
			}
			xmlwrite_three_param($ins_order, $instruction, $xw, $firstArg, $secondArg, $thirdArg);
            $stats_counter["jumps"]++;
			break;	

		default:
			fwrite(STDERR, "Error, invalid option\n"); 
			exit(22);
			break;
	}
	$ins_order++;
	$stats_counter["loc"]++;
	xmlwriter_end_element($xw);
}
xmlwriter_end_document($xw);
echo xmlwriter_output_memory($xw);
##### <----------> WRITING TO FILE <----------> #####
if (array_key_exists("stats", $options))
{
	if (($open_status = fopen($options["stats"], 'w')) == 0) 
	{
		fwrite(STDERR, "Error, file opening was unsuccessful!, check your permissions\n");
		exit(12);
	}
	else
	{
		write_to_file($options, $open_status, $stats_counter);
	}
}
?>