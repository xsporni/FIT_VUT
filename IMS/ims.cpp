/**
 * @file    ims.cpp
 * @brief   This file contains C/C++ program to simulate impact of CCS technology on certain power-plant.
 *
 * @author  Igor Mjasojedov | xmjaso00@stud.fit.vutbr.cz
 * @date    9 December 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <cstdlib> 
#include <unistd.h>
#include <math.h>
#include <iostream>
#include <string>
#include <cstring>

using namespace std;

#define EMISSION_PERMIT_COST 384.5571      // [Kc/t]

typedef struct arguments {
    long double energy_plant_year;   // -p [GWh]
    int efficiency_plant;            // -u [%]
    long double caloricity_coal;     // -v [GJ/t]
    long double emissions_coal;      // -e [kg/GJ]
    long double cost_coal;           // -c [Kc]
    int reducement_percentage;       // -r [%]
    long double emission_permit_cost;// -k [Kc]
} Arguments;

void print_help() {
    cout << "===================================== HELP message =====================================" << endl;
    cout << "-p [GWh]      The amount of electricity produced in the block of the plant in GWh" << endl;
    cout << "-u [%]        The net efficiency of the block in %" << endl;
    cout << "-v [GJ/t]     The calorific value of desired coal/lignite in GJ/t" << endl;
    cout << "-e [kg/GJ]    The amount of emissions produced from 1kg of coal/lignite in kg/GJ" << endl;
    cout << "-c [Kč]       The price of 1 tonne of coal/lignite in Kc (Czech Crown)" << endl;
    cout << "-r [%]        The desired amount of reducing emissions in %" << endl;
    cout << "-k [Kč]       The price of one emission allowance in Kc (Czech Crown)" << endl << endl;;

}

void parse_arguments(int argc, char *argv[], Arguments *arguments) {
    int opt;
    char *ptr;
    
    if(argc != 15) {
        print_help();
        exit(EXIT_FAILURE);
    }

    while((opt = getopt(argc, argv, "p:u:v:e:c:r:k:")) != -1) {
        switch(opt) {
            case 'p':
                arguments->energy_plant_year = strtod(optarg, &ptr);
                break;
            case 'u':
                arguments->efficiency_plant = strtod(optarg, &ptr);
                break;
            case 'v':
                arguments->caloricity_coal = strtod(optarg, &ptr);
                break;
            case 'e':
                arguments->emissions_coal = strtod(optarg, &ptr);
                break;
            case 'c':
                arguments->cost_coal = strtod(optarg, &ptr);
                break;         
            case 'r':
                arguments->reducement_percentage = atoi(optarg);
                break;
            case 'k':
                arguments->emission_permit_cost = strtod(optarg, &ptr);
                break;
            default:
                print_help();
                exit(EXIT_FAILURE);
        }
    }
    if(argc > optind) {
        fprintf(stderr, "Error: Too many arguments!\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    Arguments arguments;
    parse_arguments(argc, argv, &arguments);

    long double coal_coefficient = 3600.0 / ((arguments.efficiency_plant / 100.0) * arguments.caloricity_coal * 1000.0);  // [kg/1KWh]
    long double efficiency_plant_ccs = (arguments.efficiency_plant - (arguments.efficiency_plant/100.0 * 33.0 )) / 100.0;
    long double coal_coefficient_ccs = 3600.0 / ((efficiency_plant_ccs) * arguments.caloricity_coal * 1000.0);  // [kg/1KWh]
    
    long double coal_per_year = arguments.energy_plant_year * 1000.0 * coal_coefficient;  // [t]
    long double co2_per_year = coal_per_year * arguments.caloricity_coal * arguments.emissions_coal / 1000.0; // [t]
    long double co2_cost_year = coal_per_year * EMISSION_PERMIT_COST;     // [Kc]


    cout << "======================= Computed coefficients =======================" << endl;
    cout << "Amout of coal for 1MWh energy: " << coal_coefficient << "\t [tons]  (No CCS capture) " << endl;
    cout << "Amout of coal for 1MWh energy: " << coal_coefficient_ccs << "\t [tons]  (CCS capture)" << endl << endl;;
    cout << "========================== Original values ==========================" << endl;
    cout << "Coal consumption per year: \t\t"<< coal_per_year << "\t [tons]" << endl;
    cout << "Amout of CO2 released per year: \t" << co2_per_year << "\t [tons]" << endl;
    cout << "Cost of emission permit bills per year: "<< co2_cost_year << "\t [tons]" << endl;
    cout << "=====================================================================" << endl << endl;;


    double max_co2_to_release = co2_per_year - (co2_per_year*(arguments.reducement_percentage/100.0));
    cout << "Selected amout of CO2 to reduce: " << arguments.reducement_percentage << " [%]" << endl;
    cout << "  => Maximum amout of CO2 to release: " << max_co2_to_release << " [tons]" << endl << endl;

    int percentage_pointer_ccs = arguments.reducement_percentage;
    int percentage_pointer_without_ccs;
    double co2_released, co2_captured, coal_combusted_without_ccs, coal_combusted_with_ccs, emissions_coal_without_ccs, 
           emissions_coal_with_ccs, emissions_together;

    do {
    percentage_pointer_ccs++;
    percentage_pointer_without_ccs = 100 - percentage_pointer_ccs;
    coal_combusted_without_ccs = arguments.energy_plant_year * (percentage_pointer_without_ccs / 100.0) * 1000.0 * coal_coefficient; 
    coal_combusted_with_ccs = arguments.energy_plant_year * (percentage_pointer_ccs / 100.0) * 1000.0 * coal_coefficient_ccs; 

    emissions_coal_without_ccs =  coal_combusted_without_ccs * arguments.caloricity_coal * arguments.emissions_coal / 1000;
    emissions_coal_with_ccs = coal_combusted_with_ccs * arguments.caloricity_coal * arguments.emissions_coal / 1000;
    emissions_together = emissions_coal_without_ccs + emissions_coal_with_ccs;

    co2_captured = emissions_together * (percentage_pointer_ccs/100.0);
    co2_released = emissions_together - co2_captured;

    } while(co2_released > max_co2_to_release);

    cout << "============================== Results ==============================" << endl;
    cout << "Coal combusted without CCS: \t" << coal_combusted_without_ccs << "\t [tons]" << endl;
    cout << "Coal combusted with CCS: \t" << coal_combusted_with_ccs << "\t [tons]" << endl;
    cout << "  => Total coal combusted: \t" << coal_combusted_without_ccs + coal_combusted_with_ccs << "\t [tons]" << endl;
    cout << "Emissions of coal combusted without CCS: \t" << emissions_coal_without_ccs << "\t [tons]" << endl;
    cout << "Emissions of coal combusted with CCS: \t\t" << emissions_coal_with_ccs << "\t [tons]" << endl;
    cout << "Total CO2 produced: \t\t" << emissions_together << "\t [tons]" << endl;
    cout << "  => Total CO2 captured: \t" << co2_captured << "\t [tons]" << endl;
    cout << "  => Total CO2 released: \t" << co2_released << "\t [tons]" << endl << endl;

    cout << "CCS technology must be set at least to " << percentage_pointer_ccs << " % of filtering " 
         << "to decrease CO2 release by " << arguments.reducement_percentage << "% of original values." << endl << endl;

    
    double coal_amount_growth, coal_cost_growth, emissions_amount_reduction, saved_cost_of_emissions_permit_bills, balance;

    do {
        percentage_pointer_without_ccs = 100 - percentage_pointer_ccs;
        coal_combusted_without_ccs = arguments.energy_plant_year * (percentage_pointer_without_ccs / 100.0) * 1000.0 * coal_coefficient; 
        coal_combusted_with_ccs = arguments.energy_plant_year * (percentage_pointer_ccs / 100.0) * 1000.0 * coal_coefficient_ccs; 

        emissions_coal_without_ccs =  coal_combusted_without_ccs * arguments.caloricity_coal * arguments.emissions_coal / 1000;
        emissions_coal_with_ccs = coal_combusted_with_ccs * arguments.caloricity_coal * arguments.emissions_coal / 1000;
        emissions_together = emissions_coal_without_ccs + emissions_coal_with_ccs;

        co2_captured = emissions_together * (percentage_pointer_ccs/100.0);
        co2_released = emissions_together - co2_captured;

        coal_amount_growth =  (coal_combusted_without_ccs + coal_combusted_with_ccs) - coal_per_year;
        coal_cost_growth = coal_amount_growth * arguments.cost_coal;
        emissions_amount_reduction = co2_per_year - co2_released;
        saved_cost_of_emissions_permit_bills = emissions_amount_reduction * arguments.emission_permit_cost;
        balance = coal_cost_growth - saved_cost_of_emissions_permit_bills;

        percentage_pointer_ccs++;
    } while((balance > 0) && (percentage_pointer_ccs != 100));


    cout << "========================== Economic factor ==========================" << endl;
    if (percentage_pointer_ccs == 100) {
        cout << "CCS techonology is not lucrative for specified system." << endl;
    } else {
        cout << "CCS technology is lucrative when it is configured to filter "<<  percentage_pointer_ccs << "% of CO2 produced and more." << endl;
        cout << "Saved costs: " << (-balance)/1000000 << " [million Kč]" << endl;
        cout << "Total CO2 produced: \t\t" << emissions_together << "\t [tons]" << endl;
        cout << "  => Total CO2 captured: \t" << co2_captured << "\t [tons]" << endl;
        cout << "  => Total CO2 released: \t" << co2_released << "\t [tons]" << endl << endl;
    }

    return 0;
}