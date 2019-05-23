// typos and some functions used throughout moe-engine
#include "HandmadeMath.h"

#ifndef TYPOS_H
#define TYPOS_H

typedef struct KeyState {
	bool initial;
	bool down;
	bool released;
} KeyState;

typedef struct KeyStruct {
	v3 l;
	v2 r;
	
	float rot_x;
	float rot_y;
	float rot_z;
	
	KeyState g_key;
	KeyState h_key;
	KeyState j_key;
	KeyState n_key;
	KeyState m_key;
	KeyState r_key;
	KeyState t_key;
	KeyState p_key;
	
	KeyState F1_key;
	KeyState F2_key;
	KeyState F3_key;
	KeyState F4_key;
	KeyState F5_key;
	
	KeyState F10_key;
	
	double xpos;
	double ypos;
	bool in_window;
	
	KeyState left_mouse_button, right_mouse_button;
	
	int width, height;
} Key_Struct;


#define MAX_NAME_LENGTH 128


enum EditorMode {
	NEUTRAL,
	SETTINGS,
	SELECTION,
	NEW_OBJECT
};

typedef struct EditorState {
	v3 editor_point;
	v3 editor_forward;
	
	float plane_height;
	
	enum EditorMode editor_mode;
	
	int show_bvh;
	int show_all_physics_meshes;
	int show_dbg;
	
	int new_object_prototype_index;
	
	int edit_object_index;
	
	bool level_needs_saving;
	bool level_needs_loading;
	char level_load_name[MAX_NAME_LENGTH];
} EditorState;


typedef uint32_t uint;

typedef struct {
    v3 center;
    float radius;
} typ_sphere;

typedef struct {
    v3 p;
    v3 q;
} typ_segment;

typedef struct Transform {
    Quat rotation;
    v3 translation;
} Transform;

mat4 BuildProjectionMatrix(
float aspect,
float m_fovy,
float m_zNear,
float m_zFar);

v3 rotate_vec3_quat(
v3 pos,
Quat rot);

float DistPointPlane(
v3 q,
v3 p_n,
float p_d);

Transform mul_transform(Transform A, Transform B);

Quat lerp_quat(Quat Left, Quat Right, float Time);

v3 lerp_vec3(v3 Left, v3 Right, float Time);

Quat quaternion_rotation_from_vectors(v3 from, v3 to);

unsigned int loadshaders(const char* vShaderCode, const char* fShaderCode, char* infoLog);

bool compare_tail(char *to_compare, char *tail);

#endif /* TYPOS_H */

#ifdef TYPOS_IMPLEMENTATION

mat4 BuildProjectionMatrix(
float aspect,
float m_fovy,
float m_zNear,
float m_zFar)
{
    
	mat4 m;
    
    float t = tan(0.5f * m_fovy);
    float sy = 1.0f / t;
    
    float sx = 1.0f / (aspect * t);
    
    float invRange = 1.0f / (m_zNear - m_zFar);
    float sz = invRange * (m_zNear + m_zFar);
    float tz = invRange *  m_zNear * m_zFar;
    
    
    m.X = vec4(sx, 0.0f, 0.0f, 0.0f);
    m.Y = vec4(0.0f, sy, 0.0f, 0.0f);
    m.Z = vec4(0.0f, 0.0f, sz, -1.0f);
    m.W = vec4(0.0f, 0.0f, tz, 0.0f);
    return m;
}

mat4 inverse_projection_matrix(mat4 m)
{
	float u = m.X.X;
	float v = m.Y.Y;
	float d = m.Z.Z;
	float e = m.W.Z;
	
	m.X = vec4(1/u, 0.0f, 0.0f, 0.0f);
    m.Y = vec4(0.0f, 1/v, 0.0f, 0.0f);
    m.Z = vec4(0.0f, 0.0f, 0.0f, 1/e);
    m.W = vec4(0.0f, 0.0f, -1.0f, d/e);
    
    return m;
}


// divisions are kind of heavy, so maybe just store the inverse with every quaternion
// this would negatively impact the cache though, so there needs to be some testing
v3 rotate_vec3_quat(
v3 pos,
Quat rot)
{
    Quat rot_inv = inverse_quat(rot);
    Quat pos_quat = quaternion(pos.X, pos.Y, pos.Z, 0.0f);
    
    Quat res_quat = mul_quat(mul_quat(rot, pos_quat), rot_inv);
    
    return vec3(res_quat.X, res_quat.Y, res_quat.Z);
}

float DistPointPlane(
v3 q,
v3 p_n,
float p_d)
{
    return dot_v3(q, p_n) - p_d;
}

v3 ClosestPtPointPlane(
v3 q,
v3 p_n,
float p_d)
{
    float t = dot_v3(p_n, q) - p_d;
    return sub_v3(q, mul_v3f(p_n,t));
}

Transform mul_transform(Transform A, Transform B) {
    
    Transform C;
    
    C.rotation = mul_quat(A.rotation, B.rotation);
    C.translation = add_v3(rotate_vec3_quat(B.translation, A.rotation), A.translation);
    return C;
}

Quat lerp_quat(Quat Left, Quat Right, float Time)
{
    Quat Result;
    
    if (dot_quat(Left,Right) < 0) {
        Right.X = -Right.X;
        Right.Y = -Right.Y;
        Right.Z = -Right.Z;
        Right.W = -Right.W;
    }
    
    Result.X = lerp(Left.X, Time, Right.X);
    Result.Y = lerp(Left.Y, Time, Right.Y);
    Result.Z = lerp(Left.Z, Time, Right.Z);
    Result.W = lerp(Left.W, Time, Right.W);
    
    Result = normalize_quat(Result);
    
    return (Result);
}


v3 lerp_vec3(v3 Left, v3 Right, float Time)
{
    v3 Result;
    
    Result.X = lerp(Left.X, Time, Right.X);
    Result.Y = lerp(Left.Y, Time, Right.Y);
    Result.Z = lerp(Left.Z, Time, Right.Z);
    
    return Result;
}

// this has a bug in the case of opposite vectors
Quat quaternion_rotation_from_vectors(v3 from, v3 to) {
    Quat q;
    
    from = normalize_v3(from);
    to = normalize_v3(to);
    
    float dot = dot_v3(from, to);
    
    if (dot >= 1.0f) {
        return quaternion(0.0f, 0.0f, 0.0f, 1.0f);
    }
    if (dot < -0.9999) {
		return quaternion(0.0f, 1.0f, 0.0f, 0.0f);
	}
	
	
    v3 cr = cross(from, to);
    
    q.X = cr.X;
    q.Y = cr.Y;
    q.Z = cr.Z;
    q.W = 1 + dot;
    
    return normalize_quat(q);
}

void printf_vec3(v3 v) {
	printf("%f %f %f\n", v.X, v.Y, v.Z);
}

void printf_vec4(v4 v) {
	printf("%f %f %f %f\n", v.X, v.Y, v.Z, v.W);
}

void printf_matrix(mat4 m) {
	printf_vec4(m.X);
	printf_vec4(m.Y);
	printf_vec4(m.Z);
	printf_vec4(m.W);
}

// function to load shaders returns a shader program id
unsigned int loadshaders(const char* vShaderCode, const char* fShaderCode, char* infoLog) {
    
    unsigned int vertex, fragment;
    int success;
    
    // vertexShader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        printf("%s", infoLog);
        return 0;
    }
    // fragment shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    // check for shader compile errors
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        printf("%s", infoLog);
        return 0;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("%s", infoLog);
		return 0;
	}
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return shaderProgram;
}

// some parsing functions
// NOTE: all of them are unsafe, so don't parse anything that you aren't sure of 

// sets trailing newlines or spaces to \0
void clean_word(char *word) {
	int char_counter = 0;
	char *current_char = word;
	
	while(current_char != '\0') {
		if (*current_char == ' ' || *current_char == '\n' || *current_char == '\t') {
			word[char_counter] = '\0';
			return;
		}
		char_counter++;
		current_char = &word[char_counter];
	}
} 

// modifies the line, so watch out
// return the number of words in the line and stores pointers to the individual words in word_pointers
int split_line_into_words(char *line, char **word_pointers, int max_words) {
	
	int num_words = 0;
	
	int char_counter = 0;
	char *current_char = line;
	
	// consume everything until a word starts, or until there is some end signifier
	while(*current_char != '\n' || *current_char != '\0') {
		if(*current_char == ' ' || *current_char == '\t') {
			char_counter++;
			current_char = &line[char_counter];
		} else {
			word_pointers[num_words] = current_char;
			num_words++;
			char_counter++;
			current_char = &line[char_counter];
			break;
		}
	}
	
	if (max_words == 1) {
		return num_words;
	}
	
	// now break up the following words
	while(*current_char != '\n' || *current_char != '\0') {
		if (*current_char == ' ') {
			*current_char = '\0';
			word_pointers[num_words] = &line[char_counter+1];
			num_words++;
			if (num_words >= max_words) {
				return num_words;
			}
			
		}
		char_counter++;
		current_char = &line[char_counter];
	}
	
	return num_words;
};

// compares the tailing characters to the tail and returns true if both of them have the same characters
bool compare_tail(char *to_compare, char *tail) {
	
	// find end of to_compare
	int char_counter_cmp = 0;
	char *current_char_cmp = to_compare;
	while(1) {
		if (*current_char_cmp == '\0') {
			break;
		}
		char_counter_cmp++;
		current_char_cmp = &to_compare[char_counter_cmp];
	}
	
	// find end of tail
	int char_counter_tail = 0;
	char *current_char_tail = tail;
	while(1) {
		if (*current_char_tail == '\0') {
			break;
		}
		char_counter_tail++;
		current_char_tail = &tail[char_counter_tail];
	}
	
	while(char_counter_tail >= 0 && char_counter_cmp >= 0) {
		
		if(tail[char_counter_tail] != to_compare[char_counter_cmp]) {
			return false;
		}
		char_counter_cmp--;
		char_counter_tail--;
	}
	
	return true;
}


float parse_float(char *line) {
	float f = 0.0f;
	char * words[1];
	
	int num_words = split_line_into_words(line, words, 1);
	if (num_words != 1) {
		printf("number of words isn't 1 for float  %s\n", line);
		return f;
	}
	
	f = atof(words[0]);
	return f;
}


v3 parse_vec3(char *line) {
	v3 vec = vec3(0.0f, 0.0f, 0.0f);
	char * words[3];
	
	int num_words = split_line_into_words(line, words, 3);
	if (num_words != 3) {
		printf("number of words isn't 3 for vector  %s\n", line);
		return vec;
	}
	
	vec.X = atof(words[0]);
	vec.Y = atof(words[1]);
	vec.Z = atof(words[2]);
	return vec;
}

Quat parse_quaternion(char *line) {
	Quat quat = quaternion(0.0f, 0.0f, 0.0f, 1.0f);
	char * words[4];
	
	int num_words = split_line_into_words(line, words, 4);
	if (num_words != 4) {
		printf("number of words isn't 4 for quaternion  %s\n", line);
		return quat;
	}
	
	quat.X = atof(words[0]);
	quat.Y = atof(words[1]);
	quat.Z = atof(words[2]);
	quat.W = atof(words[3]);
	
	return normalize_quat(quat);
}

Quat euler_to_quaternion(float roll, float pitch, float yaw) // yaw (Z), pitch (Y), roll (X)
{
    // Abbreviations for the various angular functions
	float cy = cos(yaw * 0.5);
    float sy = sin(yaw * 0.5);
    float cp = cos(pitch * 0.5);
    float sp = sin(pitch * 0.5);
    float cr = cos(roll * 0.5);
    float sr = sin(roll * 0.5);
	
	Quat q;
    q.W = cy * cp * cr + sy * sp * sr;
    q.X = cy * cp * sr - sy * sp * cr;
    q.Y = sy * cp * sr + cy * sp * cr;
    q.Z = sy * cp * cr - cy * sp * sr;
    return q;
}

// from wikipedia https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_
void quaternion_to_euler_angle(Quat q, float *roll, float *pitch, float *yaw)
{
	// roll (x-axis rotation)
	float sinr_cosp = 2.0f * (q.W * q.X + q.Y * q.Z);
	float cosr_cosp = 1.0f - 2.0f * (q.X * q.X + q.Y * q.Y);
	*roll = HMM_ATAN2F(sinr_cosp, cosr_cosp);
	
	// pitch (y-axis rotation)
	float sinp = 2.0f * (q.W * q.Y - q.Z * q.X);
	if (HMM_ABS(sinp) >= 1.0f)
		*pitch = copysign(HMM_PI / 2.0f, sinp); // use 90 degrees if out of range
	else
		*pitch = asinf(sinp);
	
	// yaw (z-axis rotation)
	float siny_cosp = 2.0f * (q.W * q.Z + q.X * q.Y);
	float cosy_cosp = 1.0f - 2.0f * (q.Y * q.Y + q.Z * q.Z);  
	*yaw = atan2(siny_cosp, cosy_cosp);
}

#endif /* TYPOS_IMPLEMENTATION */
