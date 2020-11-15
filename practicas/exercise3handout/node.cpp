#include "Node.h"
#include "maths_funcs.h"

Node::Node()
	:parent(0)
{ ; }

void Node::init() { 
	position = vec3(0, 0, 0);
	rotation = versor(0,0,0,1);
	scale = vec3(1,1,1);

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
	mat4 T = translate( identity_mat4(), vec3( position.v[0], position.v[1], position.v[2] ) );
	mat4 R = quat_to_mat4(rotation); 
	mat4 S = scaler(identity_mat4(), scale);
	localMatrix = T*R*S;

	mat4 Tinv = translate( identity_mat4(), vec3( -position.v[0], -position.v[1], -position.v[2] ) );
	mat4 Sinv = scaler(identity_mat4(), vec3(1.f / scale.v[0], 1.f / scale.v[1], 1.f / scale.v[2]));
	localInverseMatrix = Sinv*transpose(R)*Tinv; // equivalent to tras(A)b - tras(A)t

	//localMatrix = T*R*S;
	//localInverseMatrix = Sinv*transpose(R)*Tinv;

	//localMatrix = T;
	//localInverseMatrix = Tinv;
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
