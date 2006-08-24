// Copyright (c) 2006 Fernando Luis Cacciola Carballal. All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
//
// Author(s)     : Fernando Cacciola <fernando_cacciola@ciudad.com.ar>
//
#ifndef CGAL_SURFACE_MESH_SIMPLIFICATION_POLICIES_EDGE_COLLAPSE_SET_EMPTY_COLLAPSE_DATA_H
#define CGAL_SURFACE_MESH_SIMPLIFICATION_POLICIES_EDGE_COLLAPSE_SET_EMPTY_COLLAPSE_DATA_H

#include <CGAL/Surface_mesh_simplification/TSMS_common.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Empty_collapse_data.h>

CGAL_BEGIN_NAMESPACE

namespace Triangulated_surface_mesh { namespace Simplification { namespace Edge_collapse
{

template<class TSM_>    
class Set_empty_collapse_data
{
public:

  typedef TSM_ TSM ;

  typedef typename boost::graph_traits<TSM>::edge_descriptor edge_descriptor ;
  
  typedef Empty_collapse_data<TSM> Collapse_data ;
  
  typedef void Params ;
  
public :

  void operator() ( Collapse_data&, edge_descriptor const&, TSM&, Params const* ) const {}                         
  
};    

} } } // namespace Triangulated_surface_mesh::Simplification::Edge_collapse


CGAL_END_NAMESPACE

#endif // CGAL_SURFACE_MESH_SIMPLIFICATION_POLICIES_EDGE_COLLAPSE_SET_EMPTY_COLLAPSE_DATA_H
// EOF //
 
