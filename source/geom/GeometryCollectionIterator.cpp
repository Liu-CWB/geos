#include "../headers/geom.h"
#include "../headers/util.h"
#include <typeinfo>

namespace geos {

GeometryCollectionIterator::GeometryCollectionIterator(){
	parent=NULL;
	subcollectionIterator=NULL;
	atStart=true;
    index=0;
    max=0;
}
GeometryCollectionIterator::GeometryCollectionIterator(const GeometryCollectionIterator &gci) {
	subcollectionIterator=gci.subcollectionIterator;
	parent=gci.parent;
	atStart=gci.atStart;
	index=gci.index;
	max=gci.max;
}

GeometryCollectionIterator::GeometryCollectionIterator(
		const GeometryCollection *newParent){
	parent=newParent;
	subcollectionIterator=NULL;
    atStart=true;
    index=0;
    max=newParent->getNumGeometries();
}

bool GeometryCollectionIterator::hasNext() const {
	if (atStart) {
		return true;
	}
	if (subcollectionIterator!=NULL) {
		if (subcollectionIterator->hasNext()) {
			return true;
		}
		//subcollectionIterator=NULL;
	}
	if (index>=max) {
		return false;
	}
	return true;
}

const Geometry* GeometryCollectionIterator::next() {
	// the parent GeometryCollection is the first object returned
	if (atStart) {
		atStart=false;
		return parent;
	}
	if (subcollectionIterator!=NULL) {
		if (subcollectionIterator->hasNext()) {
			return subcollectionIterator->next();
		} else {
			delete subcollectionIterator;
			subcollectionIterator=NULL; // is this automatic ?
		}
	}
	if (index>=max) {
		if ( subcollectionIterator ) delete subcollectionIterator; 
		throw new UnsupportedOperationException("No more elements");
	}
	const Geometry *obj=parent->getGeometryN(index++);
	if ((typeid(*obj)==typeid(GeometryCollection)) ||
			   (typeid(*obj)==typeid(MultiPoint)) ||
			   (typeid(*obj)==typeid(MultiLineString)) ||
			   (typeid(*obj)==typeid(MultiPolygon))) {
		subcollectionIterator=new GeometryCollectionIterator((GeometryCollection *)obj);
		// there will always be at least one element in the sub-collection
		return subcollectionIterator->next();
	}
	return obj;
}

/**
 *  Not implemented.
 *
 *@throws  UnsupportedOperationException  This method is not implemented.
 */
void GeometryCollectionIterator::remove() {
	delete subcollectionIterator;
	throw new UnsupportedOperationException();
}

GeometryCollectionIterator::~GeometryCollectionIterator(){
	delete subcollectionIterator;
}
}

