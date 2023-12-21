#include "DebugDrawing.h"




//const float PI = 3.14159;

// Creates a VAO from a list of vertices and indices
static int VaoFromPoints(std::vector<glm::vec4> Pnt, std::vector<int> Ind)
{
    unsigned int vaoID;
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    GLuint Pbuff;

    glGenBuffers(1, &Pbuff);
    glBindBuffer(GL_ARRAY_BUFFER, Pbuff);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * Pnt.size(), &Pnt[0][0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint Ibuff;

    glGenBuffers(1, &Ibuff);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Ibuff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * Ind.size(), &Ind[0], GL_STATIC_DRAW);

    glBindVertexArray(0);

    return vaoID;
}

// Makes a rotation that rotates the Z-axis to align with a given vector V.
static glm::mat4 rotateZtoV(glm::vec3 V, glm::vec3 U)
{
    glm::vec3 C = glm::normalize(V);
    glm::vec3 A = glm::normalize(glm::cross(C, U));
    glm::vec3 B = glm::cross(A, C);

    return glm::mat4(A.x, A.y, A.z, 0.0f, B.x, B.y, B.z, 0.0f, C.x, C.y, C.z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}



// Creates a VAO for an AABB bounding box
void DebugAABB::createVAO()
{
    color = { 0.9f, 0.9f, 0.9f };

    // Make standard cube verts
    std::vector<glm::vec4> verts{
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
        glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f),
        glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
    };

    // Make cube indices
    std::vector<int> indices{ 0, 1, 2, 3, 4, 5, 6, 7, 0, 4, 2, 6, 3, 7, 1, 5, 4, 6, 5, 7, 0, 2, 1, 3 };

    // Create VAO for AABB
    vaoID = VaoFromPoints(verts, indices);
}



// Sets the AABB transform
glm::mat4 DebugAABB::makeTransformFromEntity(Entity* entity)
{
    if (entity)
    {
        RenderComponent* rc = entity->getComponent<RenderComponent>();
        TransformComponent* tc = entity->getComponent<TransformComponent>();

        if (rc && tc)
        {  
            glm::mat4 transform(1.0f);

            transform = glm::translate(transform, tc->pos);
            transform = glm::rotate(transform, (tc->angle), tc->rot);
            transform = glm::scale(transform, tc->scl * glm::vec3(0.01f, 0.01f, 0.01f));

            glm::vec4 minPoint = { 9999.0f, 9999.0f, 9999.0f, 1.0f};
            glm::vec4 maxPoint = { -9999.0f, -9999.0f, -9999.0f, 1.0f };
            glm::vec3 centerPoint = { 0.0f, 0.0f, 0.0f };
            glm::vec3 dim = { 0.0f, 0.0f, 0.0f };

            //std::vector<std::vector<Vertex>>& vertices = rc->mesh->vertices;
            //std::vector < std::vector<unsigned int>>& indices = rc->mesh->indices;

            std::vector<std::vector<Vertex>> vertices;
            std::vector < std::vector<unsigned int>> indices;

            for (int i = 0; i < vertices.size(); i++)
            {
                for (int j = 0; j < vertices[i].size(); j++)
                {
                    glm::vec3& p = vertices[i][j].position;
                    glm::vec4 point = transform * glm::vec4(p.x, p.y, p.z, 1.0f);
                    
                    minPoint = glm::min(point, minPoint);
                    maxPoint = glm::max(point, maxPoint);
                }
            }

            dim.x = glm::abs(maxPoint.x - minPoint.x);
            dim.y = glm::abs(maxPoint.y - minPoint.y);
            dim.z = glm::abs(maxPoint.z - minPoint.z);

            centerPoint = glm::vec3(maxPoint.x, maxPoint.y, maxPoint.z) - dim * 0.5f;

            AABBComponent* aabb = entity->getComponent<AABBComponent>();
            
            if (aabb)
            {
                if (aabb->box)
                {
                    aabb->box->center = centerPoint;
                    aabb->box->extents = dim;
                }
            }

            // Set AABB transformation
            return Translate(glm::vec3(minPoint.x, minPoint.y, minPoint.z)) * Scale(glm::vec3(maxPoint.x, maxPoint.y, maxPoint.z) - glm::vec3(minPoint.x, minPoint.y, minPoint.z));
        }
    }
}

void DebugAABB::draw(Entity* entity, ShaderProgram* program)
{
    AABBComponent* aabb = entity->getComponent<AABBComponent>();

    if (aabb)
    {
        if (needsUpdate)
        {
            transform = makeTransformFromEntity(entity);
            needsUpdate = false;
        }


        // Set lines color for fragment shader
        int loc = glGetUniformLocation(program->programId, "diffuse");
        glUniform3fv(loc, 1, &color[0]);

        // Set transformation for vertex shader
        loc = glGetUniformLocation(program->programId, "ModelTr");
        glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(transform));



        // Draw
        glBindVertexArray(vaoID);
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}


glm::mat4 DebugGimbal::makeTransform(Entity* entity)
{
    if (entity)
    {
        RenderComponent* rc = entity->getComponent<RenderComponent>();
        TransformComponent* tc = entity->getComponent<TransformComponent>();

        if (rc && tc)
        {
            glm::vec3 A = tc->pos;
            glm::vec3 B = tc->pos + glm::vec3(0.0f, 0.0f, 5.0f);

            // Set AABB transformation
            return Translate(A) * Scale(B - A);
        }
    }
}


void DebugGimbal::draw(Entity* entity, ShaderProgram* program)
{
    if (entity)
    {
        RenderComponent* rc = entity->getComponent<RenderComponent>();
        TransformComponent* tc = entity->getComponent<TransformComponent>();

        if (rc && tc)
        {
            // Z axis
            glm::vec3 A = tc->pos;
            glm::vec3 B = tc->pos + glm::vec3(0.0f, 0.0f, 1.0f);

            transform = Translate(A) * Scale(B - A);

            int loc = glGetUniformLocation(program->programId, "ModelTr");
            glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(transform));

            loc = glGetUniformLocation(program->programId, "diffuse");
            glUniform3f(loc, 0.0f, 0.0f, 1.0f);


            glBindVertexArray(vaoID);
            glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);



            // Y axis
            A = tc->pos;
            B = tc->pos + glm::vec3(0.0f, 1.0f, 0.0f);

            transform = Translate(A) * Scale(B - A);

            loc = glGetUniformLocation(program->programId, "ModelTr");
            glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(transform));

            loc = glGetUniformLocation(program->programId, "diffuse");
            glUniform3f(loc, 0.0f, 1.0f, 0.0f);


            glBindVertexArray(vaoID);
            glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);



            // X axis
            A = tc->pos;
            B = tc->pos + glm::vec3(1.0f, 0.0f, 0.0f);

            transform = Translate(A) * Scale(B - A);

            loc = glGetUniformLocation(program->programId, "ModelTr");
            glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(transform));

            loc = glGetUniformLocation(program->programId, "diffuse");
            glUniform3f(loc, 1.0f, 0.0f, 0.0f);


            glBindVertexArray(vaoID);
            glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    }

}

void DebugGimbal::draw(glm::vec3 pos, ShaderProgram* program)
{


    // Z axis
    glm::vec3 A = pos;
    glm::vec3 B = pos + glm::vec3(0.0f, 0.0f, 1.0f);

    transform = Translate(A) * Scale(B - A);

    int loc = glGetUniformLocation(program->programId, "ModelTr");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(transform));

    loc = glGetUniformLocation(program->programId, "diffuse");
    glUniform3f(loc, 0.0f, 0.0f, 1.0f);


    glBindVertexArray(vaoID);
    glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);



    // Y axis
    A = pos;
    B = pos + glm::vec3(0.0f, 1.0f, 0.0f);

    transform = Translate(A) * Scale(B - A);

    loc = glGetUniformLocation(program->programId, "ModelTr");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(transform));

    loc = glGetUniformLocation(program->programId, "diffuse");
    glUniform3f(loc, 0.0f, 1.0f, 0.0f);


    glBindVertexArray(vaoID);
    glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);



    // X axis
    A = pos;
    B = pos + glm::vec3(1.0f, 0.0f, 0.0f);

    transform = Translate(A) * Scale(B - A);

    loc = glGetUniformLocation(program->programId, "ModelTr");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(transform));

    loc = glGetUniformLocation(program->programId, "diffuse");
    glUniform3f(loc, 1.0f, 0.0f, 0.0f);


    glBindVertexArray(vaoID);
    glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
        
    
}

void DebugGimbal::createVAO()
{
    color = { 1.0f, 0.0f, 0.0f };

    // Make standard cube verts
    std::vector<glm::vec4> verts{ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) };

    // Make cube indices
    std::vector<int> indices{ 0, 1 };

    // Create VAO for AABB
    vaoID = VaoFromPoints(verts, indices);
}





// Creates a VAO for a bounding sphere
void DebugSphere::createVAO(unsigned int N)
{
    count = N;

    std::vector<glm::vec4> verts;
    std::vector<int> indices;

    // Create sphere of radius 1 at origin
    for (unsigned int i = 0; i < N; i++)
    {
        float theta = (2.0 * PI / static_cast<float>(N)) * i;

        glm::vec4 p = { glm::cos(theta), glm::sin(theta), 0.0f, 1.0f };

        verts.push_back(p);
        indices.push_back(i);
    }

    // Create VAO for sphere
    vaoID = VaoFromPoints(verts, indices);
}




// Updates the transform for the bounding sphere
void DebugSphere::makeTransformFromEntity(Entity* entity)
{
    if (entity)
    {
        RenderComponent* rc = entity->getComponent<RenderComponent>();
        TransformComponent* tc = entity->getComponent<TransformComponent>();

        if (rc && tc)
        {
            unsigned int nVerts = rc->mesh2->Pnt.size();

            glm::mat4 transform = Translate(tc->pos) * Scale(tc->scl);

            // Find center point
            for (unsigned int i = 0; i < nVerts; i++)
            {
                glm::vec4 p = transform * rc->mesh2->Pnt[i];
                center = center + glm::vec3(p);

            }

            center /= glm::vec3(static_cast<float>(nVerts), static_cast<float>(nVerts), static_cast<float>(nVerts));

           
            float rMax = 0.0f;

            // Find radius of bounding circle
            for (unsigned int i = 0; i < nVerts; i++)
            {
                glm::vec4 p = transform * rc->mesh2->Pnt[i];
                rMax = glm::max(glm::length(center - glm::vec3(p)), rMax);
            }

            radius = rMax;

            // Set AABB transformation
            trZ = Translate(center) * Scale(glm::vec3(radius, radius, radius));
            trY = trZ * Rotate(0, 90.0f);
            trX = trZ * Rotate(1, 90.0f);

        }

    }
}


// Just updates the silhouette transform
void DebugSphere::updateSilhouette(glm::vec3 E)
{
    float d = glm::length(E - center);
    float l = glm::sqrt(d * d - radius * radius);
    float r = (l * radius) / d;
    float t = glm::sqrt(radius * radius - r * r) / d;
    glm::vec3 C = center + t * (E - center);

    trS = Translate(C) * Scale(glm::vec3(r, r, r)) * rotateZtoV(E - center, glm::vec3(1.0f, 0.0f, 0.0f));
}


// Draws the bounding sphere
void DebugSphere::draw(Entity* entity, glm::vec3 eye, ShaderProgram* program)
{
    updateSilhouette(eye);

    if (needsUpdate)
    {
        makeTransformFromEntity(entity);
        needsUpdate = false;
    }
    


    // Set lines color for fragment shader
    int loc = glGetUniformLocation(program->programId, "diffuse");
    glUniform3fv(loc, 1, &color[0]);

    // Its a debug object
    loc = glGetUniformLocation(program->programId, "objectId");
    glUniform1i(loc, 5);

    glBindVertexArray(vaoID);

    // Z circle
    loc = glGetUniformLocation(program->programId, "ModelTr");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(trZ));
    glDrawElements(GL_LINE_LOOP, count, GL_UNSIGNED_INT, 0);

    // Y circle
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(trY));
    glDrawElements(GL_LINE_LOOP, count, GL_UNSIGNED_INT, 0);

    // X circle
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(trX));
    glDrawElements(GL_LINE_LOOP, count, GL_UNSIGNED_INT, 0);

    // Silhouette circle
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(trS));
    glDrawElements(GL_LINE_LOOP, count, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}










// Creates a VAO for a single standard triangle
void DebugTriangles::createVAO()
{
    // Create standard triangle
    std::vector<glm::vec4> verts{ glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.0f) };

    // Triangle indices
    std::vector<int> indices{ 0, 1, 2 };

    // Create vao
    vaoID = VaoFromPoints(verts, indices);
}

// Draws the wireframe triangles for the referenced object
void DebugTriangles::draw(Entity* entity, ShaderProgram* program)
{
    if (entity)
    {
        RenderComponent* rc = entity->getComponent<RenderComponent>();
        TransformComponent* tc = entity->getComponent<TransformComponent>();

        if (rc && tc)
        {
            // Set lines color for fragment shader
            int loc = glGetUniformLocation(program->programId, "diffuse");
            glUniform3fv(loc, 1, &color[0]);

            // Its a debug object
            loc = glGetUniformLocation(program->programId, "objectId");
            glUniform1i(loc, 5);

            // Setting model transform
            loc = glGetUniformLocation(program->programId, "ModelTr");

            glBindVertexArray(vaoID);

            glm::mat4 transform = Translate(tc->pos) * Scale(tc->scl);

            // For every triangle in model
            unsigned int nTris = rc->mesh2->Tri.size();
            for (unsigned int i = 0; i < nTris; i++)
            {
                // Vertex indices for triangle
                int iA = rc->mesh2->Tri[i].x;
                int iB = rc->mesh2->Tri[i].y;
                int iC = rc->mesh2->Tri[i].z;

                // Triangle vertices
                glm::vec4 A = transform * rc->mesh2->Pnt[iA];
                glm::vec4 B = transform * rc->mesh2->Pnt[iB];
                glm::vec4 C = transform * rc->mesh2->Pnt[iC];

                // Set model transform
                glm::mat4 trn = { A.x, A.y, A.z, 1.0f, B.x, B.y, B.z, 1.0f, C.x, C.y, C.z, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
                glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(trn));

                // Draw
               // glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, 0);
                glDrawArrays(GL_LINE_LOOP, 0, 3);
            }

            glBindVertexArray(0);
        }
    }
}