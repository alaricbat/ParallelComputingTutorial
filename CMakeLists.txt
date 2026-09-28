cmake_minimum_required(VERSION 3.27)
project(ParallelComputingTutorial C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED COMPONENTS C)

add_executable(ParallelComputingTutorial
        C/main.c
        C/clause/clause_reduction.c
        C/clause/clause_if.c
        C/clause/clause_firstprivate.c
        C/clause/clause_shared.c
        C/clause/clause_private.c
        C/clause/clause_random.c
        C/clause/clause_array.c
        C/parallel/parallel_sequential.c
        C/parallel/parallel_direct.c
        C/parallel/parallel_for.c)

target_link_libraries(ParallelComputingTutorial
        PRIVATE OpenMP::OpenMP_C
)
