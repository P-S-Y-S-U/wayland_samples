#ifndef MESH_H
#define MESH_H

struct Mesh
{
    float* vertex_positions;
    float* vertex_texcoords;
    float* vertex_colors;

    uint16_t* indices;
};

static void GetTriangleMesh( struct Mesh* pMesh )
{
    pMesh->vertex_positions = malloc( sizeof(float) * 3 * 3 );
    pMesh->vertex_texcoords = malloc( sizeof(float) * 3 * 2 );
    pMesh->vertex_colors = malloc( sizeof(float) * 3 * 3 );
    pMesh->indices = NULL;

    // position
    pMesh->vertex_positions[0] = 0.0;
    pMesh->vertex_positions[1] = 0.5;
    pMesh->vertex_positions[2] = 0.0;

    pMesh->vertex_positions[3] = -0.5;
    pMesh->vertex_positions[4] = -0.5;
    pMesh->vertex_positions[5] = 0.0;

    pMesh->vertex_positions[6] = 0.5;
    pMesh->vertex_positions[7] = -0.5;
    pMesh->vertex_positions[8] = 0.0;

    // texcoords
    pMesh->vertex_texcoords[0] = 0.0;
    pMesh->vertex_texcoords[1] = 1.0;

    pMesh->vertex_texcoords[2] = 0.0;
    pMesh->vertex_texcoords[3] = 0.0;

    pMesh->vertex_texcoords[4] = 1.0;
    pMesh->vertex_texcoords[5] = 0.0;

    // colors
    pMesh->vertex_colors[0] = 1.0;
    pMesh->vertex_colors[1] = 0.0;
    pMesh->vertex_colors[2] = 0.0;

    pMesh->vertex_colors[3] = 0.0;
    pMesh->vertex_colors[4] = 1.0;
    pMesh->vertex_colors[5] = 0.0;

    pMesh->vertex_colors[6] = 0.0;
    pMesh->vertex_colors[7] = 0.0;
    pMesh->vertex_colors[8] = 1.0;
}

static void GetQuadMesh( struct Mesh* pMesh )
{
    pMesh->vertex_positions = malloc( sizeof(float) * 4 * 2 );
    pMesh->vertex_texcoords = malloc( sizeof(float) * 4 * 2 );
    pMesh->vertex_colors = NULL;
    pMesh->indices = malloc( sizeof(uint16_t) * 6 );

    // position
    pMesh->vertex_positions[0] = -1.0;
    pMesh->vertex_positions[1] = 1.0;

    pMesh->vertex_positions[2] = -1.0;
    pMesh->vertex_positions[3] = -1.0;

    pMesh->vertex_positions[4] = 1.0;
    pMesh->vertex_positions[5] = -1.0;

    pMesh->vertex_positions[6] = 1.0;
    pMesh->vertex_positions[7] = 1.0;

    // texcoords
    pMesh->vertex_texcoords[0] = 0.0;
    pMesh->vertex_texcoords[1] = 1.0;

    pMesh->vertex_texcoords[2] = 0.0;
    pMesh->vertex_texcoords[3] = 0.0;

    pMesh->vertex_texcoords[4] = 1.0;
    pMesh->vertex_texcoords[5] = 0.0;

    pMesh->vertex_texcoords[6] = 1.0;
    pMesh->vertex_texcoords[7] = 1.0;

    // indices
    pMesh->indices[0] = 0;
    pMesh->indices[1] = 1;
    pMesh->indices[2] = 2;
    pMesh->indices[3] = 0;
    pMesh->indices[4] = 2;
    pMesh->indices[5] = 3;
}

#endif 