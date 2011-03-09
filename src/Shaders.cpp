#include "Shaders.h"

// from www.lighthouse3d.com
char *textFileRead(const char *fn) {
	FILE *fp;
	char *content = NULL;

	int count=0;

	if (fn != NULL) {
		fp = fopen(fn,"rt");

		if (fp != NULL) {

            fseek(fp, 0, SEEK_END);
            count = ftell(fp);
            rewind(fp);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	return content;
}


void printInfoLog(GLhandleARB obj)
{
    int infologLength = 0;
    char *infoLog;

	glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB,
                                         &infologLength);

    if (infologLength > 0) {
        infoLog = new char[infologLength];
        glGetInfoLogARB(obj, infologLength, &infologLength, infoLog);
        cout << infoLog << endl;
        delete [] infoLog;
    }
}

bool setShader(GLhandleARB p, const char *srcfile, GLenum type) {
    GLhandleARB shader = glCreateShaderObjectARB(type);
	char *shaderText = textFileRead(srcfile);
    if (!shaderText) {
        cout << "File " << srcfile << " failed to load." << endl;
        return false;
    }
	glShaderSourceARB(shader, 1, (const char**)(&shaderText),NULL);
	free(shaderText);

	glCompileShaderARB(shader);
    GLint compiled = 0;
    glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
    if (!compiled) {
        printInfoLog(shader);
        return false;
    }

	glAttachObjectARB(p,shader);

    glDeleteObjectARB(shader);

    return true;
}

bool linkAndValidateShader(GLhandleARB p) {
    glLinkProgramARB(p);

    GLint linked;
    glGetObjectParameterivARB(p, GL_OBJECT_LINK_STATUS_ARB, &linked);
    if (!linked) {
        cout << "Link failed: " << endl;
        printInfoLog(p);
        return false;
    }

    glValidateProgramARB(p);
    GLint validated;
    glGetObjectParameterivARB(p, GL_OBJECT_VALIDATE_STATUS_ARB, &validated);
    if (!validated) {
        cout << "Validation failed: " << endl;
        printInfoLog(p);
        return false;
    }

    return true;
}
