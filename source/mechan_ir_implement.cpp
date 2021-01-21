#define IR_IMPLEMENT
#ifdef OMP_FOUND
	#define IR_NEURO_CRITICAL_OPENMP
#endif
#include <ir_database/ir_n2st_database.h>
#include <ir_database/ir_s2st_database.h>
#include <ir_neuro.h>
#include <ir_resource/ir_file_resource.h>

template class ir::Neuro<double, 1>;
template class ir::VectorC<double, 1>;
template class ir::MatrixC<double, 1>;
template class ir::Resource<FILE*, ir::FileIniterFreer>;
