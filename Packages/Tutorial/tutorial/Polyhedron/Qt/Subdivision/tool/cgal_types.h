///////////////////////////////////////////////////////////////////////////
//																																			 //
//	Class: My_polyhedron																								 //
//																																			 //
///////////////////////////////////////////////////////////////////////////

#ifndef	_POLYGON_MESH_
#define	_POLYGON_MESH_

#define PI 3.14159265358979323846

#ifdef _MSC_VER
#include <windows.h>
#endif

#include <GL/gl.h>

// CGAL	stuff
#include <CGAL/basic.h>
#include <CGAL/Cartesian.h>
#include <CGAL/Polyhedron_3.h>
#include <list>
#include <fstream>



// compute facet normal
struct Facet_normal	// (functor)
{
	template <class	Facet>
	void operator()(Facet& f)
	{
		typename Facet::Normal_3 sum = CGAL::NULL_VECTOR;
		typename Facet::Halfedge_around_facet_circulator h = f.facet_begin();
		do
		{
			typename Facet::Normal_3 normal	=	CGAL::cross_product(
				h->next()->vertex()->point() - h->vertex()->point(),
				h->next()->next()->vertex()->point() - h->next()->vertex()->point());
			double sqnorm	=	normal * normal;
			if(sqnorm	!= 0)
				normal = normal	/	(float)std::sqrt(sqnorm);
			sum	=	sum	+	normal;
		}
		while(++h	!= f.facet_begin());
		float	sqnorm = sum * sum;
		if(sqnorm	!= 0.0)
			f.normal() = sum / std::sqrt(sqnorm);
		else
			f.normal() = CGAL::NULL_VECTOR;
	}
};


// compute vertex	normal
struct Vertex_normal //	(functor)
{
		template <class	Vertex>
		void operator()(Vertex&	v)
		{
				typename Vertex::Normal_3	normal = CGAL::NULL_VECTOR;
				typename Vertex::Halfedge_around_vertex_circulator	pHalfedge =	v.vertex_begin();
				typename Vertex::Halfedge_around_vertex_circulator	begin =	pHalfedge;
				CGAL_For_all(pHalfedge,begin)
					if(!pHalfedge->is_border())
						normal = normal	+	pHalfedge->facet()->normal();
				float	sqnorm = normal * normal;
				if(sqnorm != 0.0f)
					v.normal() = normal	/	(float)std::sqrt(sqnorm);
				else
					v.normal() = CGAL::NULL_VECTOR;
		}
};


template <class	Refs,	class	T, class P,	class	Norm>
class	My_facet : public	CGAL::HalfedgeDS_face_base<Refs, T>
{
	// tag
	int	m_tag; 

	// normal
	Norm m_normal;

public:

	// life	cycle
	// no	constructors to	repeat,	since	only
	// default constructor mandatory

	My_facet()
	{
	}

	// tag
	const int&	tag()	{	return m_tag;	}
	void tag(const int& t)	{	m_tag	=	t; }

	// normal
	typedef	Norm Normal_3;
	Normal_3&	normal() { return	m_normal;	}
	const	Normal_3&	normal() const { return	m_normal;	}
};

template <class	Refs,	class	Tprev, class Tvertex,	class	Tface, class Norm>
class	My_halfedge	:	public CGAL::HalfedgeDS_halfedge_base<Refs,Tprev,Tvertex,Tface>
{
private:

	// tag
	int	m_tag; 

	// option	for	edge superimposing
	bool m_control_edge; 

public:

	// life	cycle
	My_halfedge()
	{
		m_control_edge = true;
	}

	// tag
	const int& tag() const { return m_tag;	}
	int& tag() { return m_tag;	}
	void tag(const int& t)	{	m_tag	=	t; }

	// edge	superimposing
	bool& control_edge()	{ return m_control_edge; }
	const bool& control_edge()	const { return m_control_edge; }
	void control_edge(const bool& flag) { m_control_edge	=	flag;	}
};


// A redefined items class for the Polyhedron_3	
// with	a	refined	vertex class that	contains a 
// member	for	the	normal vector	and	a	refined
// facet with	a	normal vector	instead	of the 
// plane equation	(this	is an	alternative	
// solution	instead	of using 
// Polyhedron_traits_with_normals_3).

template <class	Refs,	class	T, class P,	class	Norm>
class	My_vertex	:	public CGAL::HalfedgeDS_vertex_base<Refs,	T, P>
{
	// tag
	int	m_tag; 

	// normal
	Norm m_normal;

public:
	// life	cycle
	My_vertex()	 {}
	// repeat	mandatory	constructors
	My_vertex(const	P& pt)
		:	CGAL::HalfedgeDS_vertex_base<Refs, T,	P>(pt)
	{
	}

	// normal
	typedef	Norm Normal_3;
	Normal_3&	normal() { return	m_normal;	}
	const	Normal_3&	normal() const { return	m_normal;	}

	// tag
	int& tag() {	return m_tag;	}
	const int& tag() const {	return m_tag;	}
	void tag(const int& t)	{	m_tag	=	t; }
};

struct My_items	:	public CGAL::Polyhedron_items_3
{
		// wrap	vertex
		template <class	Refs,	class	Traits>
		struct Vertex_wrapper
		{
				typedef	typename Traits::Point_3	Point;
				typedef	typename Traits::Vector_3	Normal;
				typedef	My_vertex<Refs,
													CGAL::Tag_true,
													Point,
													Normal>	Vertex;
		};

		// wrap	face
		template <class	Refs,	class	Traits>
		struct Face_wrapper
		{
				typedef	typename Traits::Point_3	Point;
				typedef	typename Traits::Vector_3	Normal;
				typedef	My_facet<Refs,
												 CGAL::Tag_true,
												 Point,
												 Normal> Face;
		};

		// wrap	halfedge
		template <class	Refs,	class	Traits>
		struct Halfedge_wrapper
		{
				typedef	typename Traits::Vector_3	Normal;
				typedef	My_halfedge<Refs,
														CGAL::Tag_true,
														CGAL::Tag_true,
														CGAL::Tag_true,
														Normal>	Halfedge;
		};
};

template <class	kernel,	class	items>
class	My_polyhedron	:	public CGAL::Polyhedron_3<kernel,items>
{
public :
	typedef	typename kernel::FT	FT;
	typedef	typename kernel::Point_3 Point;
	typedef	typename kernel::Vector_3	Vector;

private	:
	// opengl	(display-lists)
	unsigned int m_display_list;
	bool m_list_done;
	bool m_modified;

	// bounding box
	FT m_min[3];
	FT m_max[3];

	// type
	bool m_pure_quad;
	bool m_pure_triangle;

public :

	// life	cycle
	My_polyhedron()	
	{
		m_modified = true;
		m_list_done	=	false;
		m_display_list = 0;
		m_pure_quad = false;
		m_pure_triangle = false;
	}
	virtual	~My_polyhedron() 
	{
		::glDeleteLists(m_display_list,1);
	}

	// modified
	const bool& modified() const { return m_modified; }
	bool& modified() { return m_modified; }
	void modified(const bool& modified) { m_modified	=	modified;	}

	// type
	bool is_pure_triangle() { return m_pure_triangle; }
	bool is_pure_quad() { return m_pure_quad; }

	// normals (per	facet, then	per	vertex)
	void compute_normals_per_facet()
	{
		std::for_each(facets_begin(),facets_end(),Facet_normal());
	}
	void compute_normals_per_vertex()
	{
		std::for_each(vertices_begin(),vertices_end(),Vertex_normal());
	}

	void compute_normals()
	{
		compute_normals_per_facet();
		compute_normals_per_vertex();
	}

	// compute bounding	box
	void compute_bounding_box(FT &xmin,
														FT &xmax,
														FT &ymin,
														FT &ymax,
														FT &zmin,
														FT &zmax)
	{
		CGAL_assertion(size_of_vertices()	>	0);
		Vertex_iterator	pVertex	=	vertices_begin();
		xmin = xmax = pVertex->point().x();
		ymin = ymax = pVertex->point().y();
		zmin = zmax = pVertex->point().z();
		for(;pVertex !=	vertices_end();pVertex++)
		{
			const Point& p = pVertex->point();
			xmin	=	std::min(xmin,p.x());
			ymin	=	std::min(ymin,p.y());
			zmin	=	std::min(zmin,p.z());
			xmax	=	std::max(xmax,p.x());
			ymax	=	std::max(ymax,p.y());
			zmax	=	std::max(zmax,p.z());
		}
		m_min[0] = xmin; m_max[0] = xmax;
		m_min[1] = ymin; m_max[1] = ymax;
		m_min[2] = zmin; m_max[2] = zmax;
	}

	// bounding box
	FT xmin() { return m_min[0]; }
	FT xmax() { return m_max[0]; }
	FT ymin() { return m_min[1]; }
	FT ymax() { return m_max[1]; }
	FT zmin() { return m_min[2]; }
	FT zmax() { return m_max[2]; }

	// copy bounding box
	void copy_bounding_box(My_polyhedron<kernel,items> *pMesh)
	{
		m_min[0] = pMesh->xmin(); m_max[0] = pMesh->xmax();
		m_min[1] = pMesh->ymin(); m_max[1] = pMesh->ymax();
		m_min[2] = pMesh->zmin(); m_max[2] = pMesh->zmax();
	}

	// degree	of a face
	static unsigned int degree(Facet_handle pFace)
	{
		return circulator_size(pFace->facet_begin());
	}

	// valence of	a	vertex
	static unsigned int valence(Vertex_handle pVertex)
	{
		return circulator_size(pVertex->vertex_begin());
	}

	// check wether	a	vertex is	on a boundary	or not
	static bool	is_border(Vertex_handle	pVertex)
	{
		Halfedge_around_vertex_circulator	pHalfEdge	=	pVertex->vertex_begin();
		if(pHalfEdge ==	NULL)	// isolated	vertex
			return true;
		Halfedge_around_vertex_circulator	d	=	pHalfEdge;
		CGAL_For_all(pHalfEdge,d)
			if(pHalfEdge->is_border())
				return true;
		return false;
	}

	// get any border	halfedge attached	to a vertex
	Halfedge_handle	get_border_halfedge(Vertex_handle	pVertex)
	{
		Halfedge_around_vertex_circulator	pHalfEdge	=	pVertex->vertex_begin();
		Halfedge_around_vertex_circulator	d	=	pHalfEdge;
		CGAL_For_all(pHalfEdge,d)
			if(pHalfEdge->next()->is_border())
				return pHalfEdge->next();
		return NULL;
	}

	// tag all halfedges
	void tag_halfedges(const int tag)
	{
		for(Halfedge_iterator pHalfedge	=	halfedges_begin();
				pHalfedge	!= halfedges_end();
				pHalfedge++)
			pHalfedge->tag(tag);
	}

	// tag all facets
	void tag_facets(const	int	tag)
	{
		for(Facet_iterator pFace	=	facets_begin();
				pFace	!= facets_end();
				pFace++)
			pFace->tag(tag);
	}

	// set index for all vertices
	void set_index_vertices()
	{
		int	index	=	0;
		for(Vertex_iterator	pVertex	=	vertices_begin();
				pVertex	!= vertices_end();
				pVertex++)
			pVertex->tag(index++);
	}

	// is pure degree ?
	bool is_pure_degree(unsigned int d)
	{
		for(Facet_iterator pFace	=	facets_begin();
				pFace	!= facets_end();
				pFace++)
			if(degree(pFace) != d)
				return false;
		return true;
	}

	// compute type
	void compute_type()
	{
		m_pure_quad = is_pure_degree(4);
		m_pure_triangle = is_pure_degree(3);
	}

	// compute facet center
	void compute_facet_center(Facet_handle pFace,
														Point& center)
	{
		Halfedge_around_facet_circulator pHalfEdge = pFace->facet_begin();
		Halfedge_around_facet_circulator end = pHalfEdge;
		Vector vec(0.0,0.0,0.0);
		int	degree = 0;
		CGAL_For_all(pHalfEdge,end)
		{
			vec	=	vec	+	(pHalfEdge->vertex()->point()-CGAL::ORIGIN);
			degree++;
    }
		center = (CGAL::ORIGIN + vec/(FT)degree);
  }

	// compute average edge length around a vertex
	FT average_edge_length_around(Vertex_handle pVertex)
	{
		FT sum = 0.0;
		Halfedge_around_vertex_circulator pHalfEdge = pVertex->vertex_begin();
		Halfedge_around_vertex_circulator end = pHalfEdge;
		Vector vec(0.0,0.0,0.0);
		int	degree = 0;
		CGAL_For_all(pHalfEdge,end)
		{
			Vector vec = pHalfEdge->vertex()->point()-
				           pHalfEdge->opposite()->vertex()->point();
			sum += std::sqrt(vec*vec);
			degree++;
		}
		return sum / (FT) degree;
	}

	// draw	using	OpenGL commands	(display lists)
	void gl_draw(bool	smooth_shading,
							 bool	use_normals)
	{
		// draw	polygons
		Facet_iterator pFacet	=	facets_begin();
		for(;pFacet	!= facets_end();pFacet++)
		{

			// begin polygon assembly
			::glBegin(GL_POLYGON);
				gl_draw_facet(pFacet,smooth_shading,use_normals);
			::glEnd(); // end polygon assembly
		}
		glFlush();
	}

	void gl_draw_facet(Facet_handle pFacet,
			                bool smooth_shading,
											bool use_normals)
	{
		// one normal	per	face
		if(use_normals &&	!smooth_shading)
		{
		  const Vector &normal = pFacet->normal();
			::glNormal3f(normal[0],normal[1],normal[2]);
		}

		// revolve around	current	face to	get	vertices
		Halfedge_around_facet_circulator pHalfedge = pFacet->facet_begin();
		do
		{
			// one normal	per	vertex
			if(use_normals &&	smooth_shading)
			{
				const Vector &normal = pHalfedge->vertex()->normal();
				::glNormal3f(normal[0],normal[1],normal[2]);
			}

			// polygon assembly	is performed per vertex
			const Point& point	=	pHalfedge->vertex()->point();
			::glVertex3d(point[0],point[1],point[2]);
		}
		while(++pHalfedge	!= pFacet->facet_begin());
	}

	// superimpose edges
	void superimpose_edges(bool	skip_ordinary_edges	=	true,
												 bool	skip_control_edges = false,
												 bool voronoi_edge = false)
	{
		::glBegin(GL_LINES);
		for(Edge_iterator h = edges_begin();
				h != edges_end();
				h++)
		{
			// ignore	this edges
			if(skip_ordinary_edges &&	!h->control_edge())
				continue;

			// ignore	control	edges
			if(skip_control_edges	&& h->control_edge())
				continue;

			if(voronoi_edge)
			{
				Facet_handle pFace1 = h->facet();
				Facet_handle pFace2 = h->opposite()->facet();
				if(pFace1 == NULL || pFace2 == NULL)
					continue;

				const Point &p1 = h->vertex()->point();
				const Point &p2 = h->next()->vertex()->point();
				const Point &p3 = h->next()->next()->vertex()->point();

				kernel k;
				Point d1 = k.construct_circumcenter_3_object()(p1,p2,p3);
				::glVertex3f(d1[0],d1[1],d1[2]);

				const Point &pp1 = h->opposite()->vertex()->point();
				const Point &pp2 = h->opposite()->next()->vertex()->point();
				const Point &pp3 = h->opposite()->next()->next()->vertex()->point();
				Point d2 = k.construct_circumcenter_3_object()(pp1,pp2,pp3);
				::glVertex3f(d2[0],d2[1],d2[2]);
			}
			else
			{
				// assembly	and	draw line	segment
				const Point& p1 = h->prev()->vertex()->point();
				const Point& p2 = h->vertex()->point();
				::glVertex3f(p1[0],p1[1],p1[2]);
				::glVertex3f(p2[0],p2[1],p2[2]);
			}
		}
		::glEnd();
	}

	// superimpose vertices
	void superimpose_vertices()
	{
		::glBegin(GL_POINTS);
		for(Point_iterator pPoint = points_begin();
				pPoint !=	points_end();
				pPoint++)
			::glVertex3f(pPoint->x(),pPoint->y(),pPoint->z());
		::glEnd(); //	// end point assembly
	}

	// superimpose vertices
	void superimpose_spheres(double scale)
	{
		GLUquadricObj* pQuadric = gluNewQuadric();
		::glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	
		for(Vertex_iterator pVertex = vertices_begin();
				pVertex !=	vertices_end();
				pVertex++)
		{
			::glPushMatrix();
			double radius = average_edge_length_around(pVertex);
			::glTranslatef(pVertex->point().x(),
				             pVertex->point().y(),
										 pVertex->point().z());
			::gluSphere(pQuadric,scale*radius,24,24); 
			::glPopMatrix();
		}
		gluDeleteQuadric(pQuadric);
	}

	// write in	obj	file format	(OBJ).
	void write_obj(char	*pFilename,
	               int incr	=	1)
	{
		std::ofstream	stream(pFilename);

		// output	vertices
		for(Point_iterator pPoint	=	points_begin();
				pPoint !=	points_end();	
				pPoint++)	
			stream <<	'v'	<< ' ' <<	pPoint->x()	<< ' ' <<
															pPoint->y()	<< ' ' <<
															pPoint->z()	<< std::endl;

		// precompute	vertex indices
		this->set_index_vertices();	

		// output	facets
		for(Facet_iterator pFacet	=	facets_begin();
				pFacet !=	facets_end();	
				pFacet++)	
		{
			stream <<	'f';
			Halfedge_around_facet_circulator pHalfedge = pFacet->facet_begin();
			do 
				stream <<	'	'	<< pHalfedge->vertex()->tag()+incr;
			while(++pHalfedge	!= pFacet->facet_begin());
			stream <<	std::endl;
		}	 
	}

	// draw bounding box
	void gl_draw_bounding_box()
	{
		::glBegin(GL_LINES);
			// along x axis
			::glVertex3f(m_min[0],m_min[1],m_min[2]);
			::glVertex3f(m_max[0],m_min[1],m_min[2]);
			::glVertex3f(m_min[0],m_min[1],m_max[2]);
			::glVertex3f(m_max[0],m_min[1],m_max[2]);
			::glVertex3f(m_min[0],m_max[1],m_min[2]);
			::glVertex3f(m_max[0],m_max[1],m_min[2]);
			::glVertex3f(m_min[0],m_max[1],m_max[2]);
			::glVertex3f(m_max[0],m_max[1],m_max[2]);
			// along y axis
			::glVertex3f(m_min[0],m_min[1],m_min[2]);
			::glVertex3f(m_min[0],m_max[1],m_min[2]);
			::glVertex3f(m_min[0],m_min[1],m_max[2]);
			::glVertex3f(m_min[0],m_max[1],m_max[2]);
			::glVertex3f(m_max[0],m_min[1],m_min[2]);
			::glVertex3f(m_max[0],m_max[1],m_min[2]);
			::glVertex3f(m_max[0],m_min[1],m_max[2]);
			::glVertex3f(m_max[0],m_max[1],m_max[2]);
			// along z axis
			::glVertex3f(m_min[0],m_min[1],m_min[2]);
			::glVertex3f(m_min[0],m_min[1],m_max[2]);
			::glVertex3f(m_min[0],m_max[1],m_min[2]);
			::glVertex3f(m_min[0],m_max[1],m_max[2]);
			::glVertex3f(m_max[0],m_min[1],m_min[2]);
			::glVertex3f(m_max[0],m_min[1],m_max[2]);
			::glVertex3f(m_max[0],m_max[1],m_min[2]);
			::glVertex3f(m_max[0],m_max[1],m_max[2]);
		::glEnd();
	}

	// count #boundaries
	unsigned int nb_boundaries()
	{
		unsigned int nb = 0;
		tag_halfedges(0);
		Halfedge_handle	seed_halfedge	=	NULL;
		while((seed_halfedge = get_border_halfedge_tag(0)) !=	NULL)
		{
			nb++;

			seed_halfedge->tag(1);
			Vertex_handle	seed_vertex	=	seed_halfedge->prev()->vertex();
			Halfedge_handle	current_halfedge = seed_halfedge;
			Halfedge_handle	next_halfedge;
			do
			{
				next_halfedge	=	current_halfedge->next();
				next_halfedge->tag(1);
				current_halfedge = next_halfedge;
			}
			while(next_halfedge->prev()->vertex()	!= seed_vertex);
		}
		return nb;
	}

	// get any border	halfedge with	tag
	Halfedge_handle	get_border_halfedge_tag(int	tag)
	{
		for(Halfedge_iterator pHalfedge	=	halfedges_begin();
				pHalfedge	!= halfedges_end();
				pHalfedge++)
			if(pHalfedge->is_border()	&&
				 pHalfedge->tag()	== tag)
				return pHalfedge;
		return NULL;
	}

	// get any facet with	tag
	Facet_handle get_facet_tag(const int tag)
	{
		for(Facet_iterator pFace	=	facets_begin();
				pFace	!= facets_end();
				pFace++)
			if(pFace->tag()	== tag)
				return pFace;
		return NULL;
	}

	// tag component 
	void tag_component(Facet_handle	pSeedFacet,
										 const int tag_free,
										 const int tag_done)
{
		pSeedFacet->tag(tag_done);
		std::list<Facet_handle> facets;
		facets.push_front(pSeedFacet);
		while(!facets.empty())
		{
			Facet_handle pFacet = facets.front();
			facets.pop_front();
			pFacet->tag(tag_done);
			Halfedge_around_facet_circulator pHalfedge = pFacet->facet_begin();
			Halfedge_around_facet_circulator end = pHalfedge;
			CGAL_For_all(pHalfedge,end)
			{
				Facet_handle pNFacet = pHalfedge->opposite()->facet();
				if(pNFacet !=	NULL && pNFacet->tag() == tag_free)
				{
					facets.push_front(pNFacet);
					pNFacet->tag(tag_done);
				}
			}
		}
}

	// count #components
	unsigned int nb_components()
	{
		unsigned int nb = 0;
		tag_facets(0);
		Facet_handle seed_facet	=	NULL;
		while((seed_facet	=	get_facet_tag(0))	!= NULL)
		{
			nb++;
			tag_component(seed_facet,0,1);
		}
		return nb;
	}

	// compute the genus
	// V - E + F + B = 2 (C	-	G)
	// C ->	#connected components
	// G : genus
	// B : #boundaries
	int	genus()
	{
		int	c	=	nb_components();
		int	b	=	nb_boundaries();
		int	v	=	size_of_vertices();
		int	e	=	size_of_halfedges()/2;
		int	f	=	size_of_facets();
		return genus(c,v,f,e,b);
	}
	int	genus(int	c,
						int	v,
						int	f,
						int	e,
						int	b)
	{
		return (2*c+e-b-f-v)/2;
	}
};


#endif //	_POLYGON_MESH_
