#include "../headers/geom.h"
#include <typeinfo>
#include "../headers/geosAlgorithm.h"

namespace geos {

Polygon::Polygon(){
	shell=new LinearRing();
	holes=new vector<Geometry *>();
}
Polygon::Polygon(const Polygon &p): Geometry(p.precisionModel, p.SRID){
	shell=new LinearRing(*p.shell);
	holes=new vector<Geometry *>();
	for(int i=0;i<(int)p.holes->size();i++) {
		LinearRing *h=new LinearRing(* (LinearRing*)(*p.holes)[i]);
		holes->push_back(h);
	}
}

Polygon::Polygon(LinearRing *newShell, PrecisionModel* precisionModel, int SRID): Geometry(precisionModel, SRID) {
	if (newShell==NULL)
		newShell=new LinearRing(CoordinateListFactory::internalFactory->createCoordinateList(), precisionModel, SRID);
	holes=new vector<Geometry *>();
	if (hasNullElements(holes)) {
		delete newShell;
		delete holes;
		throw new IllegalArgumentException("holes must not contain null elements");
	}
	if (newShell->isEmpty() && hasNonEmptyElements(holes)) {
		delete newShell;
		delete holes;
		throw new IllegalArgumentException("shell is empty but holes are not");
	}
	shell=newShell;
}

Polygon::Polygon(LinearRing *newShell, vector<Geometry *> *newHoles,
				 PrecisionModel* precisionModel, int SRID):
				Geometry(precisionModel, SRID) {
	if (newShell==NULL)
		newShell=new LinearRing(CoordinateListFactory::internalFactory->createCoordinateList(), precisionModel, SRID);

	if (newHoles==NULL)
		newHoles=new vector<Geometry *>();

	if (hasNullElements(newHoles)) {
		delete newShell;
		delete newHoles;
		throw new IllegalArgumentException("holes must not contain null elements");
	}
	if (newShell->isEmpty() && hasNonEmptyElements(newHoles)) {
		delete newShell;
		delete newHoles;
		throw new IllegalArgumentException("shell is empty but holes are not");
	}
	shell=newShell;
	holes=newHoles;
}

CoordinateList* Polygon::getCoordinates() const {
	if (isEmpty()) {
		return CoordinateListFactory::internalFactory->createCoordinateList();
	}
	CoordinateList *coordinates=CoordinateListFactory::internalFactory->createCoordinateList(getNumPoints());
	int k = -1;
	CoordinateList* shellCoordinates=shell->getCoordinates();
	for (int x = 0; x < shellCoordinates->getSize(); x++) {
		k++;
		coordinates->setAt(shellCoordinates->getAt(x),k);
	}
	delete shellCoordinates;
	for (unsigned int i = 0; i < holes->size(); i++) {
		CoordinateList* childCoordinates=((LinearRing *)(*holes)[i])->getCoordinates();
		for (int j = 0; j < childCoordinates->getSize(); j++) {
			k++;
			coordinates->setAt(childCoordinates->getAt(j),k);
		}
		delete childCoordinates;
	}
	return coordinates;
}

int Polygon::getNumPoints() const {
	int numPoints = shell->getNumPoints();
	for (unsigned int i = 0; i < holes->size(); i++) {
		numPoints += ((LinearRing *)(*holes)[i])->getNumPoints();
	}
	return numPoints;
}

int Polygon::getDimension() const {
	return 2;
}

int Polygon::getBoundaryDimension() const {
	return 1;
}

bool Polygon::isEmpty() const {
	return shell->isEmpty();
}

bool Polygon::isSimple() const {
	return true;
}

const LineString* Polygon::getExteriorRing() const {
	return shell;
}

int Polygon::getNumInteriorRing() const {
	return (int)holes->size();
}

const LineString* Polygon::getInteriorRingN(int n) const {
	return (LineString *) (*holes)[n];
}

string Polygon::getGeometryType() const {
	return "Polygon";
}

Geometry* Polygon::getBoundary() const {
	if (isEmpty()) {
		return new GeometryCollection(NULL, precisionModel, SRID);
	}
	vector<Geometry *> *rings=new vector<Geometry *>(holes->size() + 1);
	(*rings)[0]=new LineString(*shell);
	for (unsigned int i=0; i<holes->size(); i++) {
		(*rings)[i + 1] =new LineString(*(LineString*)(*holes)[i]);
	}
	return new MultiLineString(rings, precisionModel, SRID);
}

Envelope* Polygon::computeEnvelopeInternal() const {
	return shell->getEnvelopeInternal();
}

bool Polygon::equalsExact(const Geometry *other, double tolerance) const {
	if (!isEquivalentClass(other)) {
		return false;
	}
	const Polygon* otherPolygon=dynamic_cast<const Polygon*>(other);
	if (typeid(*shell)!=typeid(Geometry)) {
		return false;
	}
	Geometry* thisShell=dynamic_cast<Geometry *>(shell);
	if (typeid(*(otherPolygon->shell))!=typeid(Geometry)) {
		return false;
	}
	Geometry* otherPolygonShell=dynamic_cast<Geometry *>(otherPolygon->shell);
	if (!shell->equalsExact(otherPolygonShell, tolerance)) {
		return false;
	}
	if (holes->size()!=otherPolygon->holes->size()) {
		return false;
	}
	for (unsigned int i = 0; i < holes->size(); i++) {
		if (typeid(*(*holes)[i])!=typeid(Geometry)) {
			return false;
		}
		if (typeid(*((*(otherPolygon->holes))[i]))!=typeid(Geometry)) {
			return false;
		}
		if (!((LinearRing *)(*holes)[i])->equalsExact((*(otherPolygon->holes))[i],tolerance)) {
			return false;
		}
	}
	return true;
}

void Polygon::apply_ro(CoordinateFilter *filter) const {
	shell->apply_ro(filter);
	for (unsigned int i = 0; i < holes->size(); i++) {
		((LinearRing *)(*holes)[i])->apply_ro(filter);
	}
}

void Polygon::apply_rw(CoordinateFilter *filter) {
	shell->apply_rw(filter);
	for (unsigned int i = 0; i < holes->size(); i++) {
		((LinearRing *)(*holes)[i])->apply_rw(filter);
	}
}

void Polygon::apply_rw(GeometryFilter *filter) {
	filter->filter_rw(this);
}

void Polygon::apply_ro(GeometryFilter *filter) const {
	filter->filter_ro(this);
}

Geometry* Polygon::convexHull() const {
	return getExteriorRing()->convexHull();
}

void Polygon::normalize() {
	normalize(shell, true);
	for (unsigned int i = 0; i < holes->size(); i++) {
		normalize((LinearRing *)(*holes)[i], false);
	}
	sort(holes->begin(),holes->end(),greaterThen);
}

int Polygon::compareToSameClass(const Geometry *p) const {
	return shell->compareToSameClass(((Polygon*)p)->shell);
}

void Polygon::normalize(LinearRing *ring, bool clockwise) {
	if (ring->isEmpty()) {
		return;
	}
	CoordinateList* uniqueCoordinates=ring->getCoordinates();
	uniqueCoordinates->deleteAt(uniqueCoordinates->getSize()-1);
	const Coordinate* minCoordinate=CoordinateList::minCoordinate(ring->getCoordinates());
	CoordinateList::scroll(uniqueCoordinates, minCoordinate);
	uniqueCoordinates->add(uniqueCoordinates->getAt(0));
	ring->setPoints(uniqueCoordinates);
	if (cgAlgorithms->isCCW(ring->getCoordinates())==clockwise) {
		CoordinateList::reverse(ring->getCoordinates());
	}
}

const Coordinate* Polygon::getCoordinate() const {
	return shell->getCoordinate();
}

/**
*  Returns the area of this <code>Polygon</code>
*
*@return the area of the polygon
*/
double Polygon::getArea() const {
	double area=0.0;
	area+=fabs(CGAlgorithms::signedArea(shell->getCoordinates()));
	for(unsigned int i=0;i<holes->size();i++) {
        area-=fabs(CGAlgorithms::signedArea((*holes)[i]->getCoordinates()));
	}
	return area;
}

/**
*  Returns the perimeter of this <code>Polygon</code>
*
*@return the perimeter of the polygon
*/
double Polygon::getLength() const {
	double len=0.0;
	len+=shell->getLength();
	for(unsigned int i=0;i<holes->size();i++) {
        len+=(*holes)[i]->getLength();
	}
	return len;
}

void Polygon::apply_ro(GeometryComponentFilter *filter) const {
	filter->filter_ro(this);
	shell->apply_ro(filter);
	for(unsigned int i=0;i<holes->size();i++) {
        	(*holes)[i]->apply_ro(filter);
	}
}

void Polygon::apply_rw(GeometryComponentFilter *filter) {
	filter->filter_rw(this);
	shell->apply_rw(filter);
	for(unsigned int i=0;i<holes->size();i++) {
        	(*holes)[i]->apply_rw(filter);
	}
}

Polygon::~Polygon(){
	delete shell;
	for(int i=0;i<(int)holes->size();i++) {
		delete (*holes)[i];
	}
	delete holes;
}
}

