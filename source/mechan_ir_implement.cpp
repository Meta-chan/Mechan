#define IR_IMPLEMENT
#define IR_NEURO_CRITICAL_OPENMP
#include <ir_database/ir_n2st_database.h>
#include <ir_database/ir_s2st_database.h>
#include <ir_neuro.h>
template class ir::Neuro<double, 1>;
template class ir::VectorC<double, 1>;
template class ir::MatrixC<double, 1>;