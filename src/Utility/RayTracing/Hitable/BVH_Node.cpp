#include <RayTracing/BVH_Node.h>

#include <RayTracing/Ray.h>

#include <Utility/Math.h>

#include <algorithm>

using namespace RayTracing;
using namespace CppUtility::Other;
using namespace glm;
using namespace std;

BVH_Node::BVH_Node(Material::CPtr material)
	: box(AABB::InValid), Hitable(material) { }

BVH_Node::BVH_Node(vector<Hitable::CPtr> & hitables, Material::CPtr material)
	: box(AABB::InValid), Hitable(material){
	if (hitables.size() == 0)
		return;

	remove_if(hitables.begin(), hitables.end(), [](Hitable::CPtr hitable)->bool { return hitable->GetBoundingBox().IsValid(); });
	Build(hitables.begin(), hitables.end());
}

void BVH_Node::Build(std::vector<Hitable::CPtr>::iterator begin, std::vector<Hitable::CPtr>::iterator end){
	size_t num = end - begin;
	
	size_t axis = GetAxis(begin, end);
	sort(begin, end, [&](Hitable::CPtr a, Hitable::CPtr b)->bool {
		return a->GetBoundingBox().GetMinP()[axis] < b->GetBoundingBox().GetMinP()[axis];
	});

	if (num == 1) {
		left = *begin;
		right = NULL;
		box = left->GetBoundingBox();
		return;
	}
	
	if (num == 2) {
		left = *begin;
		right = *(begin+1);
		box = left->GetBoundingBox() + right->GetBoundingBox();
		return;
	}
	
	auto nodeLeft = new BVH_Node();
	nodeLeft->Build(begin, begin + num / 2);
	left = ToPtr(nodeLeft);

	auto nodeRight = new BVH_Node();
	nodeRight->Build(begin + num / 2, end);
	right = ToPtr(nodeRight);

	box = left->GetBoundingBox() + right->GetBoundingBox();
}

HitRst BVH_Node::RayIn(Ray::Ptr & ray) const {
	if (!box.Hit(ray))
		return false;

	HitRst * front;
	HitRst hitRstLeft = left != NULL ? left->RayIn(ray) : HitRst::InValid;
	HitRst hitRstRight = right != NULL ? right->RayIn(ray) : HitRst::InValid;

	//先进行左边的测试, 则测试后 ray.tMax 被更新(在有碰撞的情况下)
	//此时如果 hitRstRight 有效, 则可知其 ray.tMax 更小
	//故只要 hitRstRight 有效, 则说明 right 更近
	if (hitRstRight.hit)
		front = &hitRstRight;
	else if(hitRstLeft.hit)
		front = &hitRstLeft;
	else
		return HitRst::InValid;

	if (GetMat() != NULL && front->isMatCoverable) {
		front->material = GetMat();
		front->isMatCoverable = IsMatCoverable();
	}

	return *front;
}

size_t BVH_Node::GetAxis(vector<Hitable::CPtr>::const_iterator begin, const vector<Hitable::CPtr>::const_iterator end) const {
	size_t num = end - begin;
	vector<float> X, Y, Z;
	X.reserve(num);
	Y.reserve(num);
	Z.reserve(num);
	for_each(begin, end, [&](Hitable::CPtr hitable) {
		auto box = hitable->GetBoundingBox();
		auto center = box.GetCenter();
		X.push_back(center.x);
		Y.push_back(center.y);
		Z.push_back(center.z);
	});

	float varianceX = Math::Variance(X);
	float varianceY = Math::Variance(Y);
	float varianceZ = Math::Variance(Z);

	if (varianceX > varianceY) {
		if (varianceX > varianceZ)
			return 0;
		else
			return 2;
	}
	else {
		if (varianceY > varianceZ)
			return 1;
		else
			return 2;
	}
}
