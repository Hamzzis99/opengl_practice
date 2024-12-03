#version 330 core
in vec3 FragPos; //--- 위치값
in vec3 Normal; //--- 버텍스 세이더에서 받은 노멀값
out vec4 FragColor; //--- 최종 객체의 색 저장

uniform vec3 lightPos; //--- 조명의 위치
uniform vec3 lightColor; //--- 조명의 색
uniform vec3 objectColor; //--- 객체의 색

void main ()
{
	float ambientLight = 0.3f; //--- 주변 조명 계수
	vec3 ambient = ambientLight * lightColor * objectColor; //--- 주변 조명 값

	vec3 normalVector = normalize(Normal); //--- 노말값을 정규화한다.
	vec3 lightDir = normalize(lightPos - FragPos); //--- 표면과 조명의 위치로 조명의 방향을 결정한다.

	float diffuseLight = max (dot (normalVector, lightDir), 0.0); //--- N과 L의 내적 값으로 강도 조절 (음의 값을 가질 수 없게 한다.)
	
	vec3 diffuse = diffuseLight * lightColor * objectColor; //--- 산란반사조명값=산란반사값*조명색상값
	
	vec3 result = ambient + diffuse; //--- 최종 조명 설정된 픽셀 색상=(주변조명+산란반사조명)*객체 색상
	FragColor = vec4 (result, 1.0); //--- 픽셀 색을 출력
}