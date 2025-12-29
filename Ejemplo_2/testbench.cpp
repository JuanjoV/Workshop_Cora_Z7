/* Librerias estandar */
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

/* Header con la configuracion y la funcion top */
#include "dot_product_config.h"

/* Esta funcion sirve para hacer el parsing de los archivos csv */
bool parse_csv_row(const std::string &line, std::vector<int> &out) 
{
    out.clear();
    std::stringstream ss(line);
    std::string cell;

    while (std::getline(ss, cell, ',')) 
    {
        if (!cell.empty()) 
        {
            out.push_back(std::stoi(cell));
        }
    }
    return !out.empty();
}

/* El testbench siempre corresponde a la funcion main(). 
El retorno de la funcion main indicara a simulacion y cosimulacion 
si el resultado es PASS (Retorno 0) o FAIL (Cualquier valor no nulo) */
int main()
{
    /* Lectura de golden reference y golden inputs*/
    std::ifstream finputs("golden_inputs.csv");
    std::ifstream frefs("golden_references.csv");

    /* Captura si no es posible abrir alguno de los archivos. */
    if (!finputs.is_open() || !frefs.is_open())
    {
        std::cerr << "Failed to open golden files!" << std::endl;
        return 1;
    }

    std::string line_inputs, line_refs;
    /* Lee y descarta la primera linea de cada archivo, que tiene los titulos de cada columna*/
    std::getline(finputs, line_inputs);
    std::getline(frefs, line_refs);

    std::vector<int> row;
    data_t X[VECTOR_SIZE], Y[VECTOR_SIZE];

    /* Conteo de resultados */
    int passed = 0;
    int failed = 0;

    /* Bucle que itera por cada linea de ambos archivos de referencia */
    while (std::getline(finputs, line_inputs) && std::getline(frefs, line_refs))
    {
        /* Separa la linea de datos de entrada de la golden input en un vector */
        if (!parse_csv_row(line_inputs, row))
        {
            std::cerr << "Invalid input row" << std::endl;
            continue;
        }
        /* Captura si la cantidad de datos no cuadra con el tamano del vector */
        if (row.size() != 2*VECTOR_SIZE)
        {
            std::cerr << "Unexpected number of columns: " << row.size() << std::endl;
            continue;
        }

        /* Separa los datos en los vectores de entrada */
        for (int i = 0; i < VECTOR_SIZE; i++)
        {
            X[i] = row[i];
            Y[i] = row[i+VECTOR_SIZE];
        }

        /* Lee el unico dato en la golden reference*/
        o_data_t golden_reference = (o_data_t) std::stoi(line_refs);
        /* Variable del mismo tipo que la referencia para el resultado del modulo */
        o_data_t dut_result;

        /* Instancia de hardware a sintetizar */
        dot_product_hls_main(X, Y, &dut_result);

        /* Mostrar el caso que se probo */
        std::cout << "Test case:" << std::endl;
        std::cout << "X = ";
        for (int x : X) std::cout << x << " ";
        std::cout << std::endl << "Y =";
        for (int y : Y) std::cout << y << " ";
        std::cout << std::endl << "Expected = " << golden_reference << std::endl;
        std::cout << "Got = " << dut_result << std::endl;
        std::cout << "Diff = " << abs(golden_reference - dut_result) << std::endl;

        /* Criterio para contar PASS o FAIL: Si son exactamente iguales es PASS eoc, FAIL */
        if (golden_reference != dut_result)
        {
            failed += 1;
        }
        else {
            passed += 1;
        }
    }

    /* Cuando se acaban los datos de los archivos de referencia, muestra un resumen de los casos probados */
    std::cout << "=======================" << std::endl;
    std::cout << "Summary (Pass/Total):" << std::endl;
    std::cout << passed << "/" << passed+failed << std::endl;
    std::cout << "=======================" << std::endl;

    /* Retorna la cantidad de fallos. Por tanto, si falla algun caso, el test completo da FAIL.*/
    return failed;
}