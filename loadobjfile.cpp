#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <vector>

#include <map>
#include <string>

// Map to store object-specific display lists
std::map<std::string, GLuint> objectDisplayLists;


// delimiters for parsing the obj file:

#define OBJDELIMS		" \t"


struct Vertex
{
	float x, y, z;
};


struct Normal
{
	float nx, ny, nz;
};


struct TextureCoord
{
	float s, t, p;
};


struct face
{
	int v, n, t;
};


char *	ReadRestOfLine( FILE * );
void	ReadObjVTN( char *, int *, int *, int * );



int
LoadObjFile( char *name )
{
	char *cmd;		// the command string
	char *str;		// argument string

	std::vector <struct Vertex> Vertices(10000);
	std::vector <struct Normal> Normals(10000);
	std::vector <struct TextureCoord> TextureCoords(10000);

	Vertices.clear();
	Normals.clear();
	TextureCoords.clear();

	struct Vertex sv;
	struct Normal sn;
	struct TextureCoord st;


	// open the input file:

	FILE *fp = fopen( name, "r" );
	if( fp == NULL )
	{
		fprintf( stderr, "Cannot open .obj file '%s'\n", name );
		return 1;
	}


	float xmin = 1.e+37f;
	float ymin = 1.e+37f;
	float zmin = 1.e+37f;
	float xmax = -xmin;
	float ymax = -ymin;
	float zmax = -zmin;

	glBegin( GL_TRIANGLES );

	for( ; ; )
	{
		char *line = ReadRestOfLine( fp );
		if( line == NULL )
			break;


		// skip this line if it is a comment:

		if( line[0] == '#' )
			continue;


		// skip this line if it is something we don't feel like handling today:

		if( line[0] == 'g' )
			continue;

		if( line[0] == 'm' )
			continue;

		if( line[0] == 's' )
			continue;

		if( line[0] == 'u' )
			continue;


		// get the command string:

		cmd = strtok( line, OBJDELIMS );


		// skip this line if it is empty:

		if( cmd == NULL )
			continue;


		if( strcmp( cmd, "v" )  ==  0 )
		{
			str = strtok( NULL, OBJDELIMS );
			sv.x = (float)atof(str);

			str = strtok( NULL, OBJDELIMS );
			sv.y = (float)atof(str);

			str = strtok( NULL, OBJDELIMS );
			sv.z = (float)atof(str);

			Vertices.push_back( sv );

			if( sv.x < xmin )	xmin = sv.x;
			if( sv.x > xmax )	xmax = sv.x;
			if( sv.y < ymin )	ymin = sv.y;
			if( sv.y > ymax )	ymax = sv.y;
			if( sv.z < zmin )	zmin = sv.z;
			if( sv.z > zmax )	zmax = sv.z;

			continue;
		}


		if( strcmp( cmd, "vn" )  ==  0 )
		{
			str = strtok( NULL, OBJDELIMS );
			sn.nx = (float)atof( str );

			str = strtok( NULL, OBJDELIMS );
			sn.ny = (float)atof( str );

			str = strtok( NULL, OBJDELIMS );
			sn.nz = (float)atof( str );

			Normals.push_back( sn );

			continue;
		}


		if( strcmp( cmd, "vt" )  ==  0 )
		{
			st.s = st.t = st.p = 0.;

			str = strtok( NULL, OBJDELIMS );
			st.s = (float)atof( str );

			str = strtok( NULL, OBJDELIMS );
			if( str != NULL )
				st.t = (float)atof( str );

			str = strtok( NULL, OBJDELIMS );
			if( str != NULL )
				st.p = (float)atof( str );

			TextureCoords.push_back( st );

			continue;
		}


		if( strcmp( cmd, "f" )  ==  0 )
		{
			struct face vertices[10];
			for( int i = 0; i < 10; i++ )
			{
				vertices[i].v = 0;
				vertices[i].n = 0;
				vertices[i].t = 0;
			}

			int sizev = (int)Vertices.size();
			int sizen = (int)Normals.size();
			int sizet = (int)TextureCoords.size();

			int numVertices = 0;
			bool valid = true;
			int vtx = 0;
			char *str;
			while( ( str = strtok( NULL, OBJDELIMS ) )  !=  NULL )
			{
				int v, n, t;
				ReadObjVTN( str, &v, &t, &n );

				// if v, n, or t are negative, they are wrt the end of their respective list:

				if( v < 0 )
					v += ( sizev + 1 );

				if( n < 0 )
					n += ( sizen + 1 );

				if( t < 0 )
					t += ( sizet + 1 );


				// be sure we are not out-of-bounds (<vector> will abort):

				if( t > sizet )
				{
					if( t != 0 )
						fprintf( stderr, "Read texture coord %d, but only have %d so far\n", t, sizet );
					t = 0;
				}

				if( n > sizen )
				{
					if( n != 0 )
						fprintf( stderr, "Read normal %d, but only have %d so far\n", n, sizen );
					n = 0;
				}

				if( v > sizev )
				{
					if( v != 0 )
						fprintf( stderr, "Read vertex coord %d, but only have %d so far\n", v, sizev );
					v = 0;
					valid = false;
				}

				vertices[vtx].v = v;
				vertices[vtx].n = n;
				vertices[vtx].t = t;
				vtx++;

				if( vtx >= 10 )
					break;

				numVertices++;
			}


			// if vertices are invalid, don't draw anything this time:

			if( ! valid )
				continue;

			if( numVertices < 3 )
				continue;


			// list the vertices:

			int numTriangles = numVertices - 2;

			for( int it = 0; it < numTriangles; it++ )
			{
				int vv[3];
				vv[0] = 0;
				vv[1] = it + 1;
				vv[2] = it + 2;

				// get the planar normal, in case vertex normals are not defined:

				struct Vertex *v0 = &Vertices[ vertices[ vv[0] ].v - 1 ];
				struct Vertex *v1 = &Vertices[ vertices[ vv[1] ].v - 1 ];
				struct Vertex *v2 = &Vertices[ vertices[ vv[2] ].v - 1 ];

				float v01[3], v02[3], norm[3];
				v01[0] = v1->x - v0->x;
				v01[1] = v1->y - v0->y;
				v01[2] = v1->z - v0->z;
				v02[0] = v2->x - v0->x;
				v02[1] = v2->y - v0->y;
				v02[2] = v2->z - v0->z;
				Cross( v01, v02, norm );
				Unit( norm, norm );
				glNormal3fv( norm );

				for( int vtx = 0; vtx < 3 ; vtx++ )
				{
					if( vertices[ vv[vtx] ].t != 0 )
					{
						struct TextureCoord *tp = &TextureCoords[ vertices[ vv[vtx] ].t - 1 ];
						glTexCoord2f( tp->s, tp->t );
					}

					if( vertices[ vv[vtx] ].n != 0 )
					{
						struct Normal *np = &Normals[ vertices[ vv[vtx] ].n - 1 ];
						glNormal3f( np->nx, np->ny, np->nz );
					}

					struct Vertex *vp = &Vertices[ vertices[ vv[vtx] ].v - 1 ];
					glVertex3f( vp->x, vp->y, vp->z );
				}
			}
			continue;
		}


		if( strcmp( cmd, "s" )  ==  0 )
		{
			continue;
		}

	}

	glEnd();
	fclose( fp );

	fprintf( stderr, "Obj file range: [%8.3f,%8.3f,%8.3f] -> [%8.3f,%8.3f,%8.3f]\n",
		xmin, ymin, zmin,  xmax, ymax, zmax );
	fprintf( stderr, "Obj file center = (%8.3f,%8.3f,%8.3f)\n",
		(xmin+xmax)/2., (ymin+ymax)/2., (zmin+zmax)/2. );
	fprintf( stderr, "Obj file  span = (%8.3f,%8.3f,%8.3f)\n",
		xmax-xmin, ymax-ymin, zmax-zmin );

	return 0;
}



char *
ReadRestOfLine( FILE *fp )
{
	static char *line;
	std::vector<char> tmp(1000);
	tmp.clear();

	for( ; ; )
	{
		int c = getc( fp );

		if( c == EOF  &&  tmp.size() == 0 )
		{
			return NULL;
		}

		if( c == EOF  ||  c == '\n' )
		{
			delete [] line;
			line = new char [ tmp.size()+1 ];
			for( int i = 0; i < (int)tmp.size(); i++ )
			{
				line[i] = tmp[i];
			}
			line[ tmp.size() ] = '\0';	// terminating null
			return line;
		}
		else
		{
			tmp.push_back( c );
		}
	}

	return "";
}


void
ReadObjVTN( char *str, int *v, int *t, int *n )
{
	// can be one of v, v//n, v/t, v/t/n:

	if( strstr( str, "//") )				// v//n
	{
		*t = 0;
		sscanf( str, "%d//%d", v, n );
		return;
	}
	else if( sscanf( str, "%d/%d/%d", v, t, n ) == 3 )	// v/t/n
	{
		return;
	}
	else
	{
		*n = 0;
		if( sscanf( str, "%d/%d", v, t ) == 2 )		// v/t
		{
			return;
		}
		else						// v
		{
			*n = *t = 0;
			sscanf( str, "%d", v );
		}
	}
}

// Uses LoadObjFile function as a base
// Loads specific geometry from a multi-object .obj file and prepares it for use in vertex buffers.
int LoadTreeGeometry(const char *filename, const std::string &objectName, 
                     std::vector<float> &vertices, std::vector<unsigned int> &indices) {
    char *cmd;
    char *str;

    std::vector<struct Vertex> Vertices;
    std::vector<struct Normal> Normals;
    std::vector<struct TextureCoord> TextureCoords;

    struct Vertex sv;
    struct Normal sn;
    struct TextureCoord st;

    // Open the input file:
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open .obj file '%s'\n", filename);
        return 1;
    }

    bool parsingTargetObject = false;

    for (;;) {
        char *line = ReadRestOfLine(fp);
        if (line == NULL) break;

        // Skip comments:
        if (line[0] == '#') continue;

        cmd = strtok(line, OBJDELIMS);
        if (cmd == NULL) continue; // Skip empty lines

        // Look for object declaration:
        if (strcmp(cmd, "o") == 0) {
            str = strtok(NULL, OBJDELIMS);
            parsingTargetObject = (str && objectName == std::string(str));
            continue;
        }

        if (strcmp(cmd, "v") == 0) {
            str = strtok(NULL, OBJDELIMS);
            sv.x = (float)atof(str);
            str = strtok(NULL, OBJDELIMS);
            sv.y = (float)atof(str);
            str = strtok(NULL, OBJDELIMS);
            sv.z = (float)atof(str);

            Vertices.push_back(sv);
            continue;
        }

        if (strcmp(cmd, "vn") == 0) {
            str = strtok(NULL, OBJDELIMS);
            sn.nx = (float)atof(str);
            str = strtok(NULL, OBJDELIMS);
            sn.ny = (float)atof(str);
            str = strtok(NULL, OBJDELIMS);
            sn.nz = (float)atof(str);

            Normals.push_back(sn);
            continue;
        }

        if (strcmp(cmd, "vt") == 0) {
            st.s = st.t = 0.;
            str = strtok(NULL, OBJDELIMS);
            st.s = (float)atof(str);
            str = strtok(NULL, OBJDELIMS);
            if (str != NULL) st.t = (float)atof(str);

            TextureCoords.push_back(st);
            continue;
        }

        if (!parsingTargetObject) continue;

        if (strcmp(cmd, "f") == 0) {
            std::vector<unsigned int> faceIndices;
            int sizev = (int)Vertices.size();
            int sizen = (int)Normals.size();
            int sizet = (int)TextureCoords.size();

            char *str;
            while ((str = strtok(NULL, OBJDELIMS)) != NULL) {
                int v, t, n;
                ReadObjVTN(str, &v, &t, &n);

                // Handle negative indices:
                if (v < 0) v += (sizev + 1);
                if (t < 0) t += (sizet + 1);
                if (n < 0) n += (sizen + 1);

                // Validate indices:
                if (v > sizev || v <= 0) continue;
                if (t > sizet || t < 0) t = 0;
                if (n > sizen || n < 0) n = 0;

                // Store index for this face vertex
                faceIndices.push_back((unsigned int)vertices.size() / 8);

                // Add vertex attributes to vertices
                struct Vertex vert = Vertices[v - 1];
				struct Normal norm;
				if (n > 0) {
					norm = Normals[n - 1];
				} else {
					norm.nx = 0.0f;
					norm.ny = 0.0f;
					norm.nz = 0.0f;
				}
				struct TextureCoord tex;
				if (t > 0) {
					tex = TextureCoords[t - 1];
				} else {
					tex.s = 0.0f;
					tex.t = 0.0f;
				}

                vertices.push_back(vert.x);
                vertices.push_back(vert.y);
                vertices.push_back(vert.z);

                vertices.push_back(norm.nx);
                vertices.push_back(norm.ny);
                vertices.push_back(norm.nz);

                vertices.push_back(tex.s);
                vertices.push_back(tex.t);
            }

            // Triangulate the face and populate indices
            for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                indices.push_back(faceIndices[0]);
                indices.push_back(faceIndices[i]);
                indices.push_back(faceIndices[i + 1]);
            }
        }
    }

    fclose(fp);
    return 0;
}

// Uses LoadObjFile function as a base
// Loads geometry from a .obj file and prepares it for use in vertex buffers.
// Same as LoadTreeGeometry function, but does not deal with mutliple object .obj files
int LoadGeometry(const char *filename, 
                 std::vector<float> &vertices, 
                 std::vector<unsigned int> &indices) {
    char *cmd;
    char *str;

    std::vector<struct Vertex> Vertices;
    std::vector<struct Normal> Normals;
    std::vector<struct TextureCoord> TextureCoords;

    struct Vertex sv;
    struct Normal sn;
    struct TextureCoord st;

    // Open the input file
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open .obj file '%s'\n", filename);
        return 1;
    }

    for (;;) {
        char *line = ReadRestOfLine(fp);
        if (line == NULL) break;

        // Skip comments
        if (line[0] == '#') continue;

        cmd = strtok(line, OBJDELIMS);
        if (cmd == NULL) continue; // Skip empty lines

        // Process vertices
        if (strcmp(cmd, "v") == 0) {
            str = strtok(NULL, OBJDELIMS);
            sv.x = (float)atof(str);
            str = strtok(NULL, OBJDELIMS);
            sv.y = (float)atof(str);
            str = strtok(NULL, OBJDELIMS);
            sv.z = (float)atof(str);

            Vertices.push_back(sv);
            continue;
        }

        // Process vertex normals
        if (strcmp(cmd, "vn") == 0) {
            str = strtok(NULL, OBJDELIMS);
            sn.nx = (float)atof(str);
            str = strtok(NULL, OBJDELIMS);
            sn.ny = (float)atof(str);
            str = strtok(NULL, OBJDELIMS);
            sn.nz = (float)atof(str);

            Normals.push_back(sn);
            continue;
        }

        // Process texture coordinates
        if (strcmp(cmd, "vt") == 0) {
            st.s = st.t = 0.0f;
            str = strtok(NULL, OBJDELIMS);
            st.s = (float)atof(str);
            str = strtok(NULL, OBJDELIMS);
            if (str != NULL) st.t = (float)atof(str);

            TextureCoords.push_back(st);
            continue;
        }

        // Process faces (triangle indices)
        if (strcmp(cmd, "f") == 0) {
            std::vector<unsigned int> faceIndices;
            int sizev = (int)Vertices.size();
            int sizen = (int)Normals.size();
            int sizet = (int)TextureCoords.size();

            char *str;
            while ((str = strtok(NULL, OBJDELIMS)) != NULL) {
                int v, t, n;
                ReadObjVTN(str, &v, &t, &n);

                // Handle negative indices
                if (v < 0) v += (sizev + 1);
                if (t < 0) t += (sizet + 1);
                if (n < 0) n += (sizen + 1);

                // Validate indices
                if (v > sizev || v <= 0) continue;
                if (t > sizet || t < 0) t = 0;
                if (n > sizen || n < 0) n = 0;

                // Store index for this face vertex
                faceIndices.push_back((unsigned int)(vertices.size() / 8));

                // Add vertex attributes to vertices
                struct Vertex vert = Vertices[v - 1];
                struct Normal norm;
                if (n > 0) {
                    norm = Normals[n - 1];
                } else {
                    norm.nx = norm.ny = norm.nz = 0.0f;
                }

                struct TextureCoord tex;
                if (t > 0) {
                    tex = TextureCoords[t - 1];
                } else {
                    tex.s = tex.t = 0.0f;
                }

                // Push vertex data into the vertices vector
                vertices.push_back(vert.x);
                vertices.push_back(vert.y);
                vertices.push_back(vert.z);

                vertices.push_back(norm.nx);
                vertices.push_back(norm.ny);
                vertices.push_back(norm.nz);

                vertices.push_back(tex.s);
                vertices.push_back(tex.t);
            }

            // Triangulate the face and populate indices
            for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                indices.push_back(faceIndices[0]);
                indices.push_back(faceIndices[i]);
                indices.push_back(faceIndices[i + 1]);
            }
        }
    }

    fclose(fp);
    return 0;
}
