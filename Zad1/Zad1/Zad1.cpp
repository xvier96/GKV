//Jakub Góralski
//n=10, emerald
#include <GL/glut.h>
#include "glext.h"
#include <stdlib.h>
#include <stdio.h>
#include "materials.h"
#include <array>
#include "colors.h"
#include "gltools_extracted.h"

#define wglGetProcAddress glXGetProcAddress

PFNGLWINDOWPOS2IPROC glWindowPos2i = nullptr;
bool rescale_normal = false;

enum
{
	BRASS,                // mosiądz
	BRONZE,               // brąz
	POLISHED_BRONZE,      // polerowany brąz
	CHROME,               // chrom
	COPPER,               // miedź
	POLISHED_COPPER,      // polerowana miedź
	GOLD,                 // złoto
	POLISHED_GOLD,        // polerowane złoto
	PEWTER,               // grafit (cyna z ołowiem)
	SILVER,               // srebro
	POLISHED_SILVER,      // polerowane srebro
	EMERALD,              // szmaragd
	JADE,                 // jadeit
	OBSIDIAN,             // obsydian
	PEARL,                // perła
	RUBY,                 // rubin
	TURQUOISE,            // turkus
	BLACK_PLASTIC,        // czarny plastik
	BLACK_RUBBER,         // czarna guma
	NORMALS_SMOOTH,       // jeden wektor normalny na wierzchołek
	NORMALS_FLAT,         // jeden wektor normalny na ścianę
	FULL_WINDOW,          // aspekt obrazu - całe okno
	ASPECT_1_1,           // aspekt obrazu 1:1
	EXIT,                 // wyjście

	LIGHT_DIRECTIONAL,
	LIGHT_AMBIENT,
	LIGHT_SPOT
};

int aspect = FULL_WINDOW;

#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif


const int LiczbaKatow = 10;

const GLdouble left = -1.0;
const GLdouble right = 1.0;
const GLdouble bottom = -1.0;
const GLdouble top = 1.0;
const GLdouble near = 3.0;
const GLdouble far = 7.0;

GLfloat rotatex = 0.0;
GLfloat rotatey = 0.0;

int button_state = GLUT_UP;
int button_x, button_y;

GLfloat scale = 1.0;

const GLfloat *ambient = EmeraldAmbient;
const GLfloat *diffuse = EmeraldDiffuse;
const GLfloat *specular = EmeraldSpecular;
GLfloat shininess = EmeraldShininess;

int normals = NORMALS_FLAT;
//int light = LIGHT_AMBIENT;
int light = LIGHT_DIRECTIONAL;
GLfloat light_color[4] = { 0.2, 0.2, 0.2, 1.0 };
GLfloat light_position[4] =
{
	0.0,0.0,2.0,1.0
};

// kąty obrotu położenia źródła światła

GLfloat light_rotatex = 0.0;
GLfloat light_rotatey = 0.0;

GLfloat spot_direction[3] = { 0.0,0.0,-1.0 };

GLfloat spot_cutoff = 180.0;
GLfloat spot_exponent = 128.0;
GLfloat constant_attenuation = 1.0;
GLfloat linear_attenuation = 0.0;
GLfloat quadratic_attenuation = 0.0;

std::array<GLfloat, 3 * (LiczbaKatow + 2)> vertices;
std::array<int, (LiczbaKatow + 1) * 2 * 3> triangles;

void Normalize(GLfloat *v)
{
	GLfloat d = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

	if (d)
	{
		v[0] /= d;
		v[1] /= d;
		v[2] /= d;
	}
}

void Normal(GLfloat *n, int i)
{
	GLfloat v1[3], v2[3];

	// obliczenie wektorów na podstawie współrzędnych wierzchołków trójkątów
	v1[0] = vertices[3 * triangles[3 * i + 1] + 0] - vertices[3 * triangles[3 * i + 0] + 0];
	v1[1] = vertices[3 * triangles[3 * i + 1] + 1] - vertices[3 * triangles[3 * i + 0] + 1];
	v1[2] = vertices[3 * triangles[3 * i + 1] + 2] - vertices[3 * triangles[3 * i + 0] + 2];
	v2[0] = vertices[3 * triangles[3 * i + 2] + 0] - vertices[3 * triangles[3 * i + 1] + 0];
	v2[1] = vertices[3 * triangles[3 * i + 2] + 1] - vertices[3 * triangles[3 * i + 1] + 1];
	v2[2] = vertices[3 * triangles[3 * i + 2] + 2] - vertices[3 * triangles[3 * i + 1] + 2];

	// obliczenie waktora normalnego przy pomocy iloczynu wektorowego
	n[0] = v1[1] * v2[2] - v1[2] * v2[1];
	n[1] = v1[2] * v2[0] - v1[0] * v2[2];
	n[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

void DrawString(GLint x, GLint y, char *string)
{
	// położenie napisu
	glWindowPos2i(x, y);

	// wyświetlenie napisu
	int len = strlen(string);
	for (int i = 0; i < len; i++)
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, string[i]);
}

void Display()
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -(near + far) / 2);

	glRotatef(rotatex, 1.0, 0, 0);
	glRotatef(rotatey, 0, 1.0, 0);

	glScalef(scale, scale, scale);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	switch (light)
	{
	case LIGHT_AMBIENT:
		glDisable(GL_LIGHT0);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_color);
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT, GL_AMBIENT);
		break;

	case LIGHT_SPOT:
		glEnable(GL_LIGHT0);
		glDisable(GL_COLOR_MATERIAL);
		glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, spot_cutoff);
		glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, spot_exponent);
		glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, constant_attenuation);
		glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, linear_attenuation);
		glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, quadratic_attenuation);
		glPushMatrix();
		glLoadIdentity();
		glTranslatef(0, 0, -(near + far) / 2);
		glRotatef(light_rotatex, 1.0, 0, 0);
		glRotatef(light_rotatey, 0, 1.0, 0);
		glTranslatef(light_position[0], light_position[1], light_position[2]);
		glLightfv(GL_LIGHT0, GL_POSITION, light_position);
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);
		glPushAttrib(GL_LIGHTING_BIT);
		glDisable(GL_LIGHT0);
		glMaterialfv(GL_FRONT, GL_EMISSION, Red);
		glutSolidSphere(0.1, 30, 20);
		glPopAttrib();
		glPopMatrix();
		break;

	case LIGHT_DIRECTIONAL:
		glEnable(GL_LIGHT0);
		glPushMatrix();
		glLoadIdentity();
		glRotatef(light_rotatex, 1.0, 0, 0);
		glRotatef(light_rotatey, 0, 1.0, 0);
		glLightfv(GL_LIGHT0, GL_POSITION, light_position);
		glPopMatrix();
		break;
	}


	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);


	glEnable(rescale_normal ? GL_RESCALE_NORMAL : GL_NORMALIZE);

	glBegin(GL_TRIANGLES);

	if (normals == NORMALS_SMOOTH)
	{
		for (int i = 0; i < triangles.size() / 3; i++)
		{
			// obliczanie wektora normalnego dla pierwszego wierzchołka
			GLfloat n[3];
			n[0] = n[1] = n[2] = 0.0;

			// wyszukanie wszystkich ścian posiadających bie¿ący wierzchołek
			for (int j = 0; j < triangles.size() / 3; j++)
				if (3 * triangles[3 * i + 0] == 3 * triangles[3 * j + 0] ||
					3 * triangles[3 * i + 0] == 3 * triangles[3 * j + 1] ||
					3 * triangles[3 * i + 0] == 3 * triangles[3 * j + 2])
				{
					// dodawanie wektorów normalnych poszczególnych ścian
					GLfloat nv[3];
					Normal(nv, j);
					n[0] += nv[0];
					n[1] += nv[1];
					n[2] += nv[2];
				}

			// uśredniony wektor normalny jest normalizowany tylko, gdy biblioteka
			// obsługuje automatyczne skalowania jednostkowych wektorów normalnych
			if (rescale_normal == true)
				Normalize(n);
			glNormal3fv(n);
			glVertex3fv(&vertices[3 * triangles[3 * i + 0]]);

			// obliczanie wektora normalnego dla drugiego wierzchołka
			n[0] = n[1] = n[2] = 0.0;

			// wyszukanie wszystkich ścian posiadających bie¿ący wierzchołek
			for (int j = 0; j < triangles.size() / 3; j++)
				if (3 * triangles[3 * i + 1] == 3 * triangles[3 * j + 0] ||
					3 * triangles[3 * i + 1] == 3 * triangles[3 * j + 1] ||
					3 * triangles[3 * i + 1] == 3 * triangles[3 * j + 2])
				{
					// dodawanie wektorów normalnych poszczególnych ścian
					GLfloat nv[3];
					Normal(nv, j);
					n[0] += nv[0];
					n[1] += nv[1];
					n[2] += nv[2];
				}

			// uśredniony wektor normalny jest normalizowany tylko, gdy biblioteka
			// obsługuje automatyczne skalowania jednostkowych wektorów normalnych
			if (rescale_normal == true)
				Normalize(n);
			glNormal3fv(n);
			glVertex3fv(&vertices[3 * triangles[3 * i + 1]]);

			// obliczanie wektora normalnego dla trzeciego wierzchołka
			n[0] = n[1] = n[2] = 0.0;

			// wyszukanie wszystkich ścian posiadających bie¿ący wierzchołek
			for (int j = 0; j < triangles.size() / 3; j++)
				if (3 * triangles[3 * i + 2] == 3 * triangles[3 * j + 0] ||
					3 * triangles[3 * i + 2] == 3 * triangles[3 * j + 1] ||
					3 * triangles[3 * i + 2] == 3 * triangles[3 * j + 2])
				{
					// dodawanie wektorów normalnych poszczególnych ścian
					GLfloat nv[3];
					Normal(nv, j);
					n[0] += nv[0];
					n[1] += nv[1];
					n[2] += nv[2];
				}

			// uśredniony wektor normalny jest normalizowany tylko, gdy biblioteka
			// obsługuje automatyczne skalowania jednostkowych wektorów normalnych
			if (rescale_normal == true)
				Normalize(n);
			glNormal3fv(n);
			glVertex3fv(&vertices[3 * triangles[3 * i + 2]]);
		}
	}
	else
	{
		// generowanie obiektu "płaskiego" - jeden wektor normalny na ścianę
		for (int i = 0; i < triangles.size() / 3; i++)
		{
			GLfloat* a = &vertices[3 * triangles[3 * i + 0]];
			GLfloat* b = &vertices[3 * triangles[3 * i + 1]];
			GLfloat* c = &vertices[3 * triangles[3 * i + 2]];

			GLTVector3 n;
			gltGetNormalVector(a, b, c, n);

			if (rescale_normal == true)
				Normalize(n);

			glNormal3fv(n);
			glVertex3fv(a);
			glVertex3fv(b);
			glVertex3fv(c);
		}
	}

	glEnd();
	glFlush();
	glutSwapBuffers();
}

// zmiana wielkości okna

void Reshape(int width, int height)
{
	// obszar renderingu - całe okno
	glViewport(0, 0, width, height);

	// wybór macierzy rzutowania
	glMatrixMode(GL_PROJECTION);

	// macierz rzutowania = macierz jednostkowa
	glLoadIdentity();

	// parametry bryły obcinania
	if (aspect == ASPECT_1_1)
	{
		// wysokość okna większa od wysokości okna
		if (width < height && width > 0)
			glFrustum(left, right, bottom*height / width, top*height / width, near, far);
		else

			// szerokość okna większa lub równa wysokości okna
			if (width >= height && height > 0)
				glFrustum(left*width / height, right*width / height, bottom, top, near, far);
	}
	else
		glFrustum(left, right, bottom, top, near, far);

	// generowanie sceny 3D
	Display();
}

template<typename T>
T clamp(T min, T max, T value)
{
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'R':
		light_color[0] = clamp(0.0f, 1.0f, light_color[0] + 0.05f);
		break;

	case 'r':
		light_color[0] = clamp(0.0f, 1.0f, light_color[0] - 0.05f);
		break;

	case 'G':
		light_color[1] = clamp(0.0f, 1.0f, light_color[1] + 0.05f);
		break;

	case 'g':
		light_color[1] = clamp(0.0f, 1.0f, light_color[1] - 0.05f);
		break;

	case 'B':
		light_color[2] = clamp(0.0f, 1.0f, light_color[2] + 0.05f);
		break;

	case 'b':
		light_color[2] = clamp(0.0f, 1.0f, light_color[2] - 0.05f);
		break;

	case '+':
		scale += 0.05;
		break;

	case '-':
		if (scale > 0.05)
			scale -= 0.05;
		break;

	case 'S':
		if (spot_cutoff == 90)
			spot_cutoff = 180;
		else if (spot_cutoff < 90)
			spot_cutoff++;
		break;

	case 's':
		if (spot_cutoff == 180)
			spot_cutoff = 90;
		else if (spot_cutoff > 0)
			spot_cutoff--;
		break;

	case 'E':
		if (spot_exponent < 128)
			spot_exponent++;
		break;

	case 'e':
		if (spot_exponent > 0)
			spot_exponent--;
		break;

	case 'C':
		constant_attenuation += 0.1;
		break;

	case 'c':
		if (constant_attenuation > 0)
			constant_attenuation -= 0.1;
		break;

	case 'L':
		linear_attenuation += 0.1;
		break;

	case 'l':
		if (linear_attenuation > 0)
			linear_attenuation -= 0.1;
		break;

	case 'Q':
		quadratic_attenuation += 0.1;
		break;

	case 'q':
		if (quadratic_attenuation > 0)
			quadratic_attenuation -= 0.1;
		break;
	}

	// narysowanie sceny
	Display();
}

// obsługa klawiszy funkcyjnych i klawiszy kursora

void SpecialKeys(int key, int x, int y)
{
	switch (key)
	{
		// kursor w lewo
	case GLUT_KEY_LEFT:
		rotatey -= 1;
		break;

		// kursor w górę
	case GLUT_KEY_UP:
		rotatex -= 1;
		break;

		// kursor w prawo
	case GLUT_KEY_RIGHT:
		rotatey += 1;
		break;

		// kursor w dół
	case GLUT_KEY_DOWN:
		rotatex += 1;
		break;
	}

	// odrysowanie okna
	Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

// obsługa przycisków myszki

void MouseButton(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		// zapamiętanie stanu lewego przycisku myszki
		button_state = state;

		// zapamiętanie poło¿enia kursora myszki
		if (state == GLUT_DOWN)
		{
			button_x = x;
			button_y = y;
		}
	}
}

// obsługa ruchu kursora myszki

void MouseMotion(int x, int y)
{
	if (button_state == GLUT_DOWN)
	{
		rotatey += 30 * (right - left) / glutGet(GLUT_WINDOW_WIDTH) * (x - button_x);
		button_x = x;
		rotatex -= 30 * (top - bottom) / glutGet(GLUT_WINDOW_HEIGHT) * (button_y - y);
		button_y = y;
		glutPostRedisplay();
	}
}

// obsługa menu podręcznego

void Menu(int value)
{
	switch (value)
	{
		// materiał - mosiądz
	case BRASS:
		ambient = BrassAmbient;
		diffuse = BrassDiffuse;
		specular = BrassSpecular;
		shininess = BrassShininess;
		Display();
		break;

		// materiał - brąz
	case BRONZE:
		ambient = BronzeAmbient;
		diffuse = BronzeDiffuse;
		specular = BronzeSpecular;
		shininess = BronzeShininess;
		Display();
		break;

		// materiał - polerowany brąz
	case POLISHED_BRONZE:
		ambient = PolishedBronzeAmbient;
		diffuse = PolishedBronzeDiffuse;
		specular = PolishedBronzeSpecular;
		shininess = PolishedBronzeShininess;
		Display();
		break;

		// materiał - chrom
	case CHROME:
		ambient = ChromeAmbient;
		diffuse = ChromeDiffuse;
		specular = ChromeSpecular;
		shininess = ChromeShininess;
		Display();
		break;

		// materiał - miedź
	case COPPER:
		ambient = CopperAmbient;
		diffuse = CopperDiffuse;
		specular = CopperSpecular;
		shininess = CopperShininess;
		Display();
		break;

		// materiał - polerowana miedź
	case POLISHED_COPPER:
		ambient = PolishedCopperAmbient;
		diffuse = PolishedCopperDiffuse;
		specular = PolishedCopperSpecular;
		shininess = PolishedCopperShininess;
		Display();
		break;

		// materiał - złoto
	case GOLD:
		ambient = GoldAmbient;
		diffuse = GoldDiffuse;
		specular = GoldSpecular;
		shininess = GoldShininess;
		Display();
		break;

		// materiał - polerowane złoto
	case POLISHED_GOLD:
		ambient = PolishedGoldAmbient;
		diffuse = PolishedGoldDiffuse;
		specular = PolishedGoldSpecular;
		shininess = PolishedGoldShininess;
		Display();
		break;

		// materiał - grafit (cyna z ołowiem)
	case PEWTER:
		ambient = PewterAmbient;
		diffuse = PewterDiffuse;
		specular = PewterSpecular;
		shininess = PewterShininess;
		Display();
		break;

		// materiał - srebro
	case SILVER:
		ambient = SilverAmbient;
		diffuse = SilverDiffuse;
		specular = SilverSpecular;
		shininess = SilverShininess;
		Display();
		break;

		// materiał - polerowane srebro
	case POLISHED_SILVER:
		ambient = PolishedSilverAmbient;
		diffuse = PolishedSilverDiffuse;
		specular = PolishedSilverSpecular;
		shininess = PolishedSilverShininess;
		Display();
		break;

		// materiał - szmaragd
	case EMERALD:
		ambient = EmeraldAmbient;
		diffuse = EmeraldDiffuse;
		specular = EmeraldSpecular;
		shininess = EmeraldShininess;
		Display();
		break;

		// materiał - jadeit
	case JADE:
		ambient = JadeAmbient;
		diffuse = JadeDiffuse;
		specular = JadeSpecular;
		shininess = JadeShininess;
		Display();
		break;

		// materiał - obsydian
	case OBSIDIAN:
		ambient = ObsidianAmbient;
		diffuse = ObsidianDiffuse;
		specular = ObsidianSpecular;
		shininess = ObsidianShininess;
		Display();
		break;

		// materiał - perła
	case PEARL:
		ambient = PearlAmbient;
		diffuse = PearlDiffuse;
		specular = PearlSpecular;
		shininess = PearlShininess;
		Display();
		break;

		// metariał - rubin
	case RUBY:
		ambient = RubyAmbient;
		diffuse = RubyDiffuse;
		specular = RubySpecular;
		shininess = RubyShininess;
		Display();
		break;

		// materiał - turkus
	case TURQUOISE:
		ambient = TurquoiseAmbient;
		diffuse = TurquoiseDiffuse;
		specular = TurquoiseSpecular;
		shininess = TurquoiseShininess;
		Display();
		break;

		// materiał - czarny plastik
	case BLACK_PLASTIC:
		ambient = BlackPlasticAmbient;
		diffuse = BlackPlasticDiffuse;
		specular = BlackPlasticSpecular;
		shininess = BlackPlasticShininess;
		Display();
		break;

		// materiał - czarna guma
	case BLACK_RUBBER:
		ambient = BlackRubberAmbient;
		diffuse = BlackRubberDiffuse;
		specular = BlackRubberSpecular;
		shininess = BlackRubberShininess;
		Display();
		break;

		// wektory normalne - GLU_SMOOTH
	case NORMALS_SMOOTH:
		normals = NORMALS_SMOOTH;
		Display();
		break;

		// wektory normalne - GLU_FLAT
	case NORMALS_FLAT:
		normals = NORMALS_FLAT;
		Display();
		break;

		// obszar renderingu - całe okno
	case FULL_WINDOW:
		aspect = FULL_WINDOW;
		Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		break;

		// obszar renderingu - aspekt 1:1
	case ASPECT_1_1:
		aspect = ASPECT_1_1;
		Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		break;

	case LIGHT_AMBIENT:
	case LIGHT_DIRECTIONAL:
	case LIGHT_SPOT:
		light = value;
		Display();
		break;
		// wyjście
	case EXIT:
		exit(0);
	}
}

// sprawdzenie i przygotowanie obsługi wybranych rozszerzeñ

void ExtensionSetup()
{
	// pobranie numeru wersji biblioteki OpenGL
	const char *version = (char*)glGetString(GL_VERSION);

	// odczyt wersji OpenGL
	int major = 0, minor = 0;
	if (sscanf_s(version, "%d.%d", &major, &minor) != 2)
	{
		printf("Bledny format wersji OpenGL\n");

		exit(0);
	}

	// sprawdzenie czy jest co najmniej wersja 1.2
	if (major > 1 || minor >= 2)
		rescale_normal = true;
	else if (glutExtensionSupported("GL_EXT_rescale_normal"))
		rescale_normal = true;

	// sprawdzenie czy jest co najmniej wersja 1.4
	if (major > 1 || minor >= 4)
	{
		// pobranie wskaźnika wybranej funkcji OpenGL 1.4
		//glWindowPos2i = (PFNGLWINDOWPOS2IPROC)wglGetProcAddress("glWindowPos2i");
	}
	else if (glutExtensionSupported("GL_ARB_window_pos"))
	{
		// pobranie wskaźnika wybranej funkcji rozszerzenia ARB_window_pos
		//glWindowPos2i = (PFNGLWINDOWPOS2IPROC)wglGetProcAddress("glWindowPos2iARB");
	}
	else
	{
		printf("Brak rozszerzenia ARB_window_pos!\n");
		exit(0);
	}
}

void BuildPyramid()
{
	vertices[3 * LiczbaKatow + 0] = 0.0f;
	vertices[3 * LiczbaKatow + 1] = 0.0f;
	vertices[3 * LiczbaKatow + 2] = 0.0f;
	vertices[3 * (LiczbaKatow + 1) + 0] = 0.0f;
	vertices[3 * (LiczbaKatow + 1) + 1] = 0.0f;
	vertices[3 * (LiczbaKatow + 1) + 2] = 1.0f;

	// podstawa
	for (int i = 0; i < LiczbaKatow; ++i)
	{
		const GLfloat x = 0.5f * std::sin(2.0f * 3.1415f * ((GLfloat)i / LiczbaKatow));
		const GLfloat y = 0.5f * std::cos(2.0f * 3.1415f * ((GLfloat)i / LiczbaKatow));

		vertices[3 * i + 0] = x;
		vertices[3 * i + 1] = y;
		vertices[3 * i + 2] = 0.0f;
	}

	// górne ściany boczne
	for (int i = 0; i <= LiczbaKatow; ++i)
	{
		triangles[3 * i + 0] = LiczbaKatow;
		triangles[3 * i + 1] = i % LiczbaKatow;
		triangles[3 * i + 2] = (i + 1) % LiczbaKatow;

		triangles[3 * (i + LiczbaKatow + 1) + 0] = (i + 1) % LiczbaKatow;
		triangles[3 * (i + LiczbaKatow + 1) + 1] = i % LiczbaKatow;
		triangles[3 * (i + LiczbaKatow + 1) + 2] = LiczbaKatow + 1;
	}
}

int main(int argc, char *argv[])
{
	BuildPyramid();

	// inicjalizacja biblioteki GLUT
	glutInit(&argc, argv);

	// inicjalizacja bufora ramki
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// rozmiary głównego okna programu
	glutInitWindowSize(500, 500);

	// utworzenie głównego okna programu
	glutCreateWindow("Wektory normalne");

	// dołączenie funkcji generującej scenę 3D
	glutDisplayFunc(Display);

	// dołączenie funkcji wywoływanej przy zmianie rozmiaru okna
	glutReshapeFunc(Reshape);

	// dołączenie funkcji obsługi klawiatury
	glutKeyboardFunc(Keyboard);

	// dołączenie funkcji obsługi klawiszy funkcyjnych i klawiszy kursora
	glutSpecialFunc(SpecialKeys);

	// obsługa przycisków myszki
	glutMouseFunc(MouseButton);

	// obsługa ruchu kursora myszki
	glutMotionFunc(MouseMotion);

	// utworzenie menu podręcznego
	glutCreateMenu(Menu);

	// utworzenie podmenu - Materiał
	int MenuMaterial = glutCreateMenu(Menu);

	glutAddMenuEntry("Mosiadz", BRASS);
	glutAddMenuEntry("Braz", BRONZE);
	glutAddMenuEntry("Polerowany braz", POLISHED_BRONZE);
	glutAddMenuEntry("Chrom", CHROME);
	glutAddMenuEntry("Miedz", COPPER);
	glutAddMenuEntry("Polerowana miedz", POLISHED_COPPER);
	glutAddMenuEntry("Zloto", GOLD);
	glutAddMenuEntry("Polerowane zloto", POLISHED_GOLD);
	glutAddMenuEntry("Grafit (cyna z olowiem)", PEWTER);
	glutAddMenuEntry("Srebro", SILVER);
	glutAddMenuEntry("Polerowane srebro", POLISHED_SILVER);
	glutAddMenuEntry("Szmaragd", EMERALD);
	glutAddMenuEntry("Jadeit", JADE);
	glutAddMenuEntry("Obsydian", OBSIDIAN);
	glutAddMenuEntry("Perla", PEARL);
	glutAddMenuEntry("Rubin", RUBY);
	glutAddMenuEntry("Turkus", TURQUOISE);
	glutAddMenuEntry("Czarny plastik", BLACK_PLASTIC);
	glutAddMenuEntry("Czarna guma", BLACK_RUBBER);

	int MenuNormals = glutCreateMenu(Menu);
	glutAddMenuEntry("Jeden wektor normalny na wierzcholek", NORMALS_SMOOTH);
	glutAddMenuEntry("Jeden wektor normalny na sciane", NORMALS_FLAT);

	int MenuAspect = glutCreateMenu(Menu);
	glutAddMenuEntry("Aspekt obrazu - cale okno", FULL_WINDOW);
	glutAddMenuEntry("Aspekt obrazu 1:1", ASPECT_1_1);

	int MenuLight = glutCreateMenu(Menu);
	glutAddMenuEntry("Kierunkowe", LIGHT_DIRECTIONAL);
	glutAddMenuEntry("Otaczajace", LIGHT_AMBIENT);
	glutAddMenuEntry("Reflektor", LIGHT_SPOT);

	glutCreateMenu(Menu);

	glutAddSubMenu("Material", MenuMaterial);
	glutAddSubMenu("Swiatlo", MenuLight);
	glutAddSubMenu("Wektory normalne", MenuNormals);
	glutAddSubMenu("Aspekt obrazu", MenuAspect);
	glutAddMenuEntry("Wyjscie", EXIT);

	glutAttachMenu(GLUT_RIGHT_BUTTON);

	ExtensionSetup();

	glutMainLoop();
	return 0;
}