#include "Node.h"
#include "maths_funcs.h"

Node::Node()
	:parent(0)
{ ; }

void Node::init() { 

	position = vec3(0, 0, 0);
	rotation = versor(0, 0, 0, 1);
	scale = vec3(1, 1, 1);

	localMatrix = identity_mat4();
	worldMatrix = identity_mat4();
	localInverseMatrix = identity_mat4();
	worldInverseMatrix = identity_mat4();
}

void Node::addChild(Node& node) { 
	node.parent = this;
	children.push_back(&node);
}

void Node::removeChild(Node& node) { 
	auto it = std::find(children.begin(), children.end(), &node);
	if (it != children.end()) {
		node.parent = nullptr;
		children.erase(it);
	}
}

void  Node::updateLocal() 
{ 
	
	vec3 positionInv = vec3(-position.x, -position.y, -position.z);
	vec3 scaleInv = vec3(-scale.x, -scale.y, -scale.z);

	mat4 matrixT = translate(identity_mat4(), position);

	mat4 matrixR = quat_to_mat4(rotation);

	mat4 matrixS = scaler(identity_mat4(), scale);

	mat4 matrixTinv = translate(identity_mat4(), positionInv);
	mat4 matrixRinv = transpose(matrixR);
	mat4 matrixSinv = scaler(identity_mat4(), scaleInv);

	localMatrix = matrixT * matrixR * matrixS;
	localInverseMatrix = matrixSinv * matrixRinv * matrixTinv;
}

void  Node::updateHierarchy()
{
	updateLocal();
	mat4 parentMatrix = identity_mat4();
	mat4 parentInverseMatrix = identity_mat4();
	if (parent!= nullptr) {
		parentMatrix = (*parent).worldMatrix;
		parentInverseMatrix = (*parent).worldInverseMatrix;
	}
	worldMatrix = parentMatrix*localMatrix;
	worldInverseMatrix = parentInverseMatrix*localInverseMatrix;

	for (size_t i = 0; i < children.size(); ++i) {
		Node* child = children[i];
		if (child != nullptr) {
			(*child).updateHierarchy();
		}
	}
}
