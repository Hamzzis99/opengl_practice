#version 330 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
out vec3 FragPos; //--- 객체의 위치값을 프래그먼트 세이더로 보낸다.
out vec3 Normal; //--- 노멀값을 프래그먼트 세이더로 보낸다.
uniform mat4 model; //--- 모델링 변환값
uniform mat4 view; //--- 뷰잉 변환값
uniform mat4 projection; //--- 투영 변환값

void main()
{
	gl_Position = projection * view * model * vec4(vPos, 1.0);
	FragPos = vec3(model * vec4(vPos, 1.0)); //--- 객체에 대한 조명 계산을 프래그먼트 셰이더에서 한다. 
	//--- 따라서 월드공간에 있는 버텍스 값을 프래그먼트 셰이더로 보낸다.

	Normal = vec3(transpose(inverse(model)) * vec4(vNormal, 1.0)); //--- 노멀값을 프래그먼트 세이더로 보낸다.
}