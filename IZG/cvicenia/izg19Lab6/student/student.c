/*!
 * @file 
 * @brief This file contains implementation of exercise.
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 * @author Tomáš Chlubna, ichlubna@fit.vutbr.cz
 *
 */

#include<assert.h>
#include<math.h>

#include<student/student.h>
#include<student/linearAlgebra.h>
#include<student/camera.h>
#include<student/model.h>
#include<student/mouseCamera.h>
#include<opengl/opengl.h>
#include<student/program.h>

///This variable contains projection matrix.
extern Mat4 projectionMatrix;
///This variable contains view matrix.
extern Mat4 viewMatrix;
///This variable contains camera position in world-space.
extern Vec3 cameraPosition;

/**
 * @brief This structure contains all global variables for this method.
 */
struct PhongVariables{
  ///This variable contains light poistion in world-space.
  Vec3 lightPosition;
  ///This variable contains a program id.
  GLuint program;
  ///This variable contains a location of projection matrix uniform.
  GLint  projectionMatrixUniform;
  ///This variable contains a location of view matrix uniform.
  GLint  viewMatrixUniform;
  ///This variable contains a location of camera position uniform.
  GLint  cameraPositionUniform;
  ///This variable contains a location of light position uniform.
  GLint  lightPositionUniform;
  ///This variable contains a vertex arrays object id.
  GLuint vao;
  ///This variable contains a buffer id for vertex attributes.
  GLuint vbo;
  ///This variable contains a buffer id for vertex indices.
  GLuint ebo;
}phong;///<instance of all global variables for triangle example.

/// \addtogroup vs Vertex Shader
/// @{

/// \addtogroup task2 Druhý úkol
/// @{
/// \todo 2.1.) Doimplementujte vertex shader.
/// Vašim úkolem je přidat uniformní proměnné pro view a projekční matici.
/// Dále pronásobte pozici vrcholu těmito maticemi a zapište výsledek do gl_Position.
/// Nezapomeňte, že píšete v jazyce GLSL, který umožňuje práci s maticovými a vektorovými typy.
/// Upravujte phongVertexShaderSource proměnnou.
/// @}

/// \addtogroup task3 Třetí úkol
/// @{
/// \todo 3.1.) Upravte vertex shader.
/// Vašim úkolem je přidat druhý vertex atribut - normálu vrcholu.
/// Dále přidejte dvě výstupní proměnné typu vec3 a zapište do nich pozici a normálu vrcholu ve world-space.
/// Tyto proměnné budete potřebovat pro výpočet osvětlení ve fragment shaderu.
/// Upravujte phongVertexShaderSource proměnnou.
/// @}

/// This variable contains vertex shader source for phong shading/lighting.
char const*phongVertexShaderSource = 
"#version 330\n"
"layout(location=0)in vec3 position;\n"



"void main(){\n"
"  gl_Position = vec4(position,1.f);\n"



"}\n";
/// @}

/// \addtogroup fs Fragment Shader
/// @{

/// \addtogroup task3 Třetí úkol
/// @{
/// \todo 3.2.) Upravte fragment shader (proměnná phongFragmentShaderSource).
/// Vašim úkolem je implementovat phongův osvětlovací model.
/// Přidejte dvě vstupní proměnné (typ vec3) stejného názvu, jako nově přidané výstupní proměnné ve vertex shaderu.
/// V jedné obdržíte pozice fragmentu ve world-space.
/// V druhé obdržíte normálu fragmentu ve world-space.
/// Dále přidejte dvě uniformní proměnné (typ vec3) pro pozici kamery a pro pozici světla.
/// Difuzní barvu materiálu nastavte podle zadání konkrétní skupiny cvičení, ptejte se pokud nevíte jak má materiál vypadat
/// Spekulání barvu materiálu nastavte na vec3(1.f,1.f,1.f) - bílá.<br/>
/// Shininess faktor nastavte na 40.f.<br/>
/// Předpokládejte, že světlo má bílou barvu.<br/>
/// Barva se vypočítá podle vzorce dF*dM*dL + sF*sM*sL.<br/>
/// dM,sM jsou difuzní/spekulární barvy materiálu - vektory.<br/>
/// dL,sL jsou difuzní/spekulární barvy světla - vektory.<br/>
/// dF,sF jsou difuzní/spekulární faktory - skaláry.<br/>
/// dF lze vypočíst pomocí vztahu clamp(dot(N,L),0.f,1.f) - skalární součin a ořez do rozsahu [0,1].<br/>
/// N je normála fragmentu (nezapomeňte ji normalizovat).<br/>
/// L je normalizovaný vektor z pozice fragmentu směrem ke světlu.<br/>
/// sF lze vypočíst pomocí vztahu pow((clamp(dot(R,L),0.f,1.f)),s) - skalární součin, ořez do rozsahu [0,1] a umocnění.<br/>
/// s je shininess faktor.<br/>
/// R je odražený pohledový vektor V; R = reflect(V,N).<br/>
/// V je normalizovaný pohledový vektor z pozice kamery do pozice fragmentu.<br/>
/// <br/>
/// Nezapomeňte, že programujete v jazyce GLSL, který zvládá vektorové operace.<br/>
/// <b>Seznam užitečných funkcí:</b>
///  - dot(x,y) - funkce vrací skalární součin dvou vektorů x,y stejné délky
///  - clamp(x,a,b) - funkce vrací ořezanou hodnotu x do intervalu [a,b]
///  - normalize(x) - funkce vrací normalizovaný vektor x
///  - reflect(I,N) - funkce vrací odražený vektor I podle normály N
///  - pow(x,y) - funkce vrací umocnění x na y - x^y
/// @}

/// This variable contains fragment shader source for phong shading/linghting.
char const*phongFragmentShaderSource = 
"#version 330\n"
"layout(location=0)out vec4 fColor;\n"


"void main(){\n"
"  fColor = vec4(1.f);\n"


"}\n";
/// @}

/// \addtogroup init Inicializace
/// @{
void phong_onInit(int32_t width,int32_t height){
  //init matrices
  cpu_initMatrices(width,height);
  //init lightPosition
  init_Vec3(&phong.lightPosition,100.f,100.f,100.f);

  GLuint const vertexId = compileShader(
      GL_VERTEX_SHADER       , //a type of shader
      phongVertexShaderSource);//a shader source
  GLuint const fragmentId = compileShader(
      GL_FRAGMENT_SHADER       , //a type of shader
      phongFragmentShaderSource);//a shader source
  phong.program = linkProgram(
      vertexId,fragmentId);

  /// \addtogroup task1 První úkol
  /// @{
  /// \todo 1.1.) Doprogramujte inicializační funkci phong_onInit().
  /// Zde byste měli vytvořit buffery na GPU, nahrát data do bufferů, vytvořit vertex arrays object a správně jej nakonfigurovat.
  /// Do bufferů nahrajte vrcholy králička (pozice, normály) a indexy na vrcholy ze souboru model.h.
  /// Využijte proměnné ve struktuře PhongVariables (vbo, ebo, vao).
  /// Do proměnné phong.vbo zapište id bufferu obsahující vertex atributy.
  /// Do proměnné phong.ebo zapište id bufferu obsahující indexy na vrcholy.
  /// Do proměnné phong.vao zapište id vertex arrays objektu.
  /// Data vertexů naleznete v proměnné model.h/modelVertices - ty překopírujte do bufferu phong.vbo.
  /// Data indexů naleznete v proměnné model.h/modelIndices - ty překopírujte do bufferu phong.ebo.
  /// Dejte si pozor, abyste správně nastavili stride a offset
  /// Vrchol králička je složen ze dvou vertex atributů: pozice a normála.
  /// Prozatím zkuste pouze nahrát pozice vertexů
  /// Použijte DSA přístup, viz upozornění v závorkách níže
  /// <b>Seznam funkcí, které jistě využijete:</b>
  ///  - glCreateBuffers (namísto starého glGenBuffers)
  ///  - glNamedBufferData (namísto starého glBufferData a glBindBuffer)
  ///  - glCreateVertexArrays (namísto starého glGenVertexArrays)
  ///  - glVertexArrayElementBuffer
  ///  - glVertexArrayVertexBuffer (namísto starého glGetAttribLocation a glVertexAttribPointer)
  ///  - glEnableVertexArrayAttrib
  ///  - glBindVertexArray
  ///  @}

  /// \addtogroup task2 Druhý úkol
  /// @{
  /// \todo 2.2.) Ve funkci phong_onInit() získejte lokace přidaných uniformních proměnných pro projekční a view matice.
  /// Zapište lokace do příslušných položek ve struktuře PhongVariables.
  /// Nezapomeňte, že lokace získáte pomocí jména proměnné v jazyce GLSL, které jste udělali v předcházejícím kroku.
  /// <b>Seznam funkcí, které jistě využijete:</b>
  ///  - glGetUniformLocation
  /// @}
  
  /// \addtogroup task3 Třetí úkol
  /// @{
  /// \todo 3.3.) Ve funkci phong_onInit() získejte lokace přidaných uniformních proměnných pro pozici světla a pro pozice kamery.
  /// Zapište lokace do příslušných položek ve struktuře PhongVariables.
  /// Nezapomeňte, že lokace získáte pomocí jména proměnné v jazyce GLSL, které jste udělali v předcházejícím kroku.<br/>
  /// <b>Seznam funkcí, které jistě využijete:</b>
  ///  - glGetUniformLocation
  /// @}


  /// \addtogroup task3 Třetí úkol
  /// @{
  /// \todo 3.4.) Ve funkci phong_onInit() nastavte druhý vertex atribut pro normálu podobně jako pro pozici.
  /// Musíte získat lokaci vstupní proměnné ve vertex shaderu, kterou jste přidali v předcházejícím kroku.
  /// Musíte správně nastavit stride a offset - normála nemá nulový offset.<br/>
  /// @}



  glClearColor(.1f,.1f,.1f,1.f);
  glEnable(GL_DEPTH_TEST);

}

/// @}

void phong_onExit(){
  glDeleteBuffers(1,&phong.vbo);
  glDeleteBuffers(1,&phong.ebo);
  glDeleteVertexArrays(1,&phong.vao);
  glDeleteProgram(phong.program);
}

/// \addtogroup draw Kreslení
/// @{

void phong_onDraw(){
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  glUseProgram(phong.program);

  /// \addtogroup task1 První úkol
  /// @{
  /// \todo 1.2.) Doprogramujte kreslící funkci phong_onDraw().
  /// Zde byste měli aktivovat vao a spustit kreslení.
  /// Funcke glDrawElements kreslí indexovaně, vyžaduje 4 parametry: mode - typ primitia, počet indexů, 
  /// typ indexů (velikost indexu), a offset.
  /// Kreslíte trojúhelníky, počet vrcholů odpovídá počtu indexů viz proměnná model.h/modelIndices.<br/>
  /// <b>Seznam funkcí, které jistě využijete:</b>
  ///  (- glBindVertexArray)
  ///  - glDrawElements
  /// @}


  /// \addtogroup task2 Druhý úkol
  /// @{
  /// \todo 2.3.) Upravte funkci phong_onDraw().
  /// Nahrajte data matic na grafickou kartu do uniformních proměnných.
  /// Aktuální data matic naleznete v externích proměnných viewMatrix a projectionMatrix.
  /// <b>Seznam funkcí, které jistě využijete:</b>
  ///  - glUniformMatrix4fv
  /// @}

  /// \addtogroup task3 Třetí úkol
  /// @{
  /// \todo 3.5.) Ve funkci phong_onDraw() nahrajte pozici světla a pozici kamery na GPU.
  /// Pozice světla a pozice kamery je v proměnných phong.lightPosition a cameraPosition.<br/>
  /// <b>Seznam funkcí, které jistě využijete:</b>
  ///  - glUniform3f nebo glUniform3fv
  /// @}


}

/// @}


