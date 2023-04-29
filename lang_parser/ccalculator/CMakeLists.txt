project(ccalculator)

set(HEADERS
    list.h
    token.h
    parser.h
    mpool.h
    type.h
)

set(SRCS
    parser.c
    token.c
    calculator.c
    mpool.c
    type.c
    ${HEADERS}
)

add_executable(ccalculator ${SRCS})