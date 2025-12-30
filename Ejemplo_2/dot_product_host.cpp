/* Librerias estandar */
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <chrono>
#include <cmath>
#include <limits>

#define VECTOR_SIZE 10
#define N_TESTS 100

typedef int32_t data_t;
typedef int32_t o_data_t;




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

/* Función de producto punto */
void dot_product_ref(data_t X[VECTOR_SIZE], data_t Y[VECTOR_SIZE], o_data_t* result)
{
    o_data_t sum = 0;
    for (int i = 0; i < VECTOR_SIZE; i++) 
    {
        sum += X[i] * Y[i];
    }
    *result = sum;
}


int main()
{
    /* Lectura de golden reference y golden inputs*/
    std::ifstream finputs("golden_inputs.csv");
    std::ifstream frefs("golden_references.csv");
    static size_t iter_cnt = 0;
    static double sum_ns = 0.0;
    static double sumsq_ns = 0.0;
    static double tmin_ns = std::numeric_limits<double>::infinity();
    static double tmax_ns = 0.0;

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

        /* Mide el tiempo de ejecucion de la funcion de referencia 100 000 veces para que sea visible por el chrono*/
        const int TIMING_REPEATS = 100000;
        o_data_t tmp;
        volatile o_data_t prevent_opt = 0; // evita optimizaciones que eliminen el bucle

        auto t_start = std::chrono::high_resolution_clock::now();
        for (int r = 0; r < TIMING_REPEATS; ++r) {
            dot_product_ref(X, Y, &tmp);
            prevent_opt ^= tmp;
        }
        auto t_end = std::chrono::high_resolution_clock::now();

        // Obtener el resultado real una vez (para la comprobación)
        dot_product_ref(X, Y, &dut_result);

        double dur_ns = std::chrono::duration<double, std::nano>(t_end - t_start).count() / TIMING_REPEATS;
        (void)prevent_opt;


        ++iter_cnt;
        sum_ns += dur_ns;
        sumsq_ns += dur_ns * dur_ns;
        if (dur_ns < tmin_ns) tmin_ns = dur_ns;
        if (dur_ns > tmax_ns) tmax_ns = dur_ns;

        /* Tiempo por iteracion */
        std::cout << "Iteration time (ns): " << dur_ns << std::endl;

        /* Mostrar el caso que se probo */
        std::cout << "Test case: " << iter_cnt-1 << std::endl;
        std::cout << "X = ";
        for (int x : X) std::cout << x << " ";
        std::cout << std::endl << "Y =";
        for (int y : Y) std::cout << y << " ";
        std::cout << std::endl << "Expected = " << golden_reference << std::endl;
        std::cout << "Got = " << dut_result << std::endl;
        std::cout << "Diff = " << abs(golden_reference - dut_result) << std::endl;

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

    double avg = sum_ns / iter_cnt;
    double variance = (sumsq_ns / iter_cnt) - (avg * avg);
    double stddev = variance > 0.0 ? std::sqrt(variance) : 0.0;

    std::cout << "----- Timing summary (nanoseconds) -----" << std::endl;
    std::cout << "Iterations: " << iter_cnt << std::endl;
    std::cout << "Average: " << avg << std::endl;
    std::cout << "Stddev: " << stddev << std::endl;
    std::cout << "Min: " << tmin_ns << std::endl;
    std::cout << "Max: " << tmax_ns << std::endl;
    std::cout << "----------------------------------------" << std::endl;

 
    return failed;
}