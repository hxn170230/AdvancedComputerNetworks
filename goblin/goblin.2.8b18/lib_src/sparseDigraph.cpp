
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, December 1998
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   sparseDigraph.cpp
/// \brief  Constructor methods for sparse directed graph objects

#include "sparseDigraph.h"


diGraph::diGraph(TNode _n,goblinController& _CT,bool _mode) throw() :
    managedObject(_CT),
    abstractDiGraph(_n,TArc(0)),
    X(static_cast<const diGraph&>(*this))
{
    X.SetCDemand(0);
    X.SetCOrientation(1);
    mode = _mode;

    LogEntry(LOG_MEM,"...Sparse digraph instanciated");
}


diGraph::diGraph(const char* fileName,goblinController& _CT) throw(ERFile,ERParse) :
    managedObject(_CT),
    abstractDiGraph(TNode(0),TArc(0)),
    X(static_cast<const diGraph&>(*this))
{
    #if defined(_TIMERS_)

    CT.globalTimer[TimerIO] -> Enable();

    #endif

    LogEntry(LOG_IO,"Loading digraph...");
    if (!CT.logIO && CT.logMem) LogEntry(LOG_MEM,"Loading digraph...");

    goblinImport F(fileName,CT);

    F.Scan("digraph");
    ReadAllData(F);

    if (CT.sourceNode<n) SetSourceNode(CT.sourceNode);
    else SetSourceNode(NoNode);

    if (CT.targetNode<n) SetTargetNode(CT.targetNode);
    else SetTargetNode(NoNode);

    if (CT.rootNode<n) SetRootNode(CT.rootNode);
    else SetRootNode(NoNode);

    X.SetCOrientation(1);

    int l = strlen(fileName)-4;
    char* tmpLabel = new char[l+1];
    memcpy(tmpLabel,fileName,l);
    tmpLabel[l] = 0;
    SetLabel(tmpLabel);
    delete[] tmpLabel;
    CT.SetMaster(Handle());

    #if defined(_TIMERS_)

    CT.globalTimer[TimerIO] -> Disable();

    #endif
}


diGraph::diGraph(abstractMixedGraph& G,TOption options) throw() :
    managedObject(G.Context()),
    abstractDiGraph(G.N(),TArc(0)),
    X(*this)
{
    X.SetCOrientation(1);

    X.SetCapacity(G.N(),2*G.M());

    TIndex* originalArc = (options & OPT_MAPPINGS) ? new TArc[2*G.M()] : NULL;

    if (options & OPT_CLONE)
    {
        for (TNode u=0;u<n;u++)
        {
            X.SetDemand(u,G.Demand(u));

            for (TDim i=0;i<G.Dim();i++) X.SetC(u,i,G.C(u,i));
        }

        for (TArc a=0;a<G.M();a++)
        {
            TNode u = G.StartNode(2*a);
            TNode v = G.EndNode(2*a);
            TCap tmpCap = (options & OPT_SUB) ? TCap(G.Sub(2*a)) : G.UCap(2*a);

            TArc a1 = InsertArc(u,v,tmpCap,G.Length(2*a),G.LCap(2*a));

            if (originalArc) originalArc[a1] = 2*a;
        }

        for (TNode u=0;u<n;u++)
        {
            TArc a = G.First(u);

            if (a==NoArc) continue;

            do
            {
                TArc a2 = G.Right(a,u);
                X.SetRight(a,a2);
                a = a2;
            }
            while (a!=G.First(u));

            X.SetFirst(u,a);
        }

        if (G.ExteriorArc()!=NoArc)
        {
            face = new TNode[2*m];

            for (TArc i=0;i<2*m;i++) face[i] = G.Face(i);

            SetExteriorArc(G.ExteriorArc());
        }

        LogEntry(LOG_MEM,"...Digraph clone generated");
    }
    else
    {
        LogEntry(LOG_MAN,"Computing orientation...");

        TNode* adjacent = new TNode[n];
        for (TNode w=0;w<n;w++) adjacent[w] = NoNode;

        THandle H = G.Investigate();
        for (TNode u=0;u<n;u++)
        {
            X.SetDemand(u,G.Demand(u));

            for (TDim i=0;i<G.Dim();i++) X.SetC(u,i,G.C(u,i));

            while (G.Active(H,u))
            {
                TArc a = G.Read(H,u);
                TNode v = G.EndNode(a);
                TCap tmpCap = (options & OPT_SUB) ? TCap(G.Sub(a)) : G.UCap(a);

                if (tmpCap>0 && !G.Blocking(a) && ((options
                    & OPT_PARALLELS) || adjacent[v]!=u))
                {
                    TArc a1 = NoArc;

                    if (G.IsBipartite())
                    {
                        if (u>v) continue; 

                        a1 = InsertArc(u,v,tmpCap,G.Length(a&(a^1)),G.LCap(a));
                    }
                    else
                    {
                        a1 = InsertArc(u,v,tmpCap,G.Length(a&(a^1)),G.LCap(a));
                    }

                    adjacent[v] = u;

                    if (originalArc) originalArc[a1] = a;
                }
            }
        }
        G.Close(H);

        delete[] adjacent;

        X.SetCapacity(N(),M());
    }

    if (options & OPT_MAPPINGS)
    {
        TIndex* originalArcExport = registers.RawArray<TIndex>(*this,TokRegOriginalArc);
        memcpy(originalArcExport,originalArc,sizeof(TIndex)*m);
        delete[] originalArc;
    }

    if (CT.traceLevel==2) Display();
}


diGraph::~diGraph() throw()
{
    LogEntry(LOG_MEM,"...Sparse digraph disallocated");

    if (CT.traceLevel==2 && mode==0) Display();
}


unsigned long diGraph::Size() const throw()
{
    return 
          sizeof(diGraph)
        + managedObject::Allocated()
        + abstractMixedGraph::Allocated()
        + abstractDiGraph::Allocated();
}


unsigned long diGraph::Allocated() const throw()
{
    return 0;
}


completeOrientation::completeOrientation(abstractMixedGraph &G,TOption options) throw() :
    managedObject(G.Context()),
    diGraph(G.N(),G.Context(),true)
{
    LogEntry(LOG_MAN,"Orienting graph arcs...");

    X.SetCapacity(G.N(),2*G.M());

    for (TArc a=0;a<2*G.M();a++)
    {
        if (!G.Blocking(a) || (options & OPT_REVERSE))
            InsertArc(G.StartNode(a),G.EndNode(a),G.UCap(a),G.Length(a&(a^1)));
    }

    X.SetCapacity(N(),M());

    for (TNode v=0;v<n;v++)
    {
        X.SetDemand(v,G.Demand(v));

        for (TDim i=0;i<G.Dim();i++) X.SetC(v,i,G.C(v,i));
    }

    type = 2;

    if (G.IsDirected()) type = 0;
    if (G.IsUndirected()) type = 1;

    if (type==2)
    {
        origin = new TArc[m];
        TArc i = 0;

        for (TArc a=0;a<2*G.M();a++)
            if (!G.Blocking(a) || (options & OPT_REVERSE)) origin[i++] = a;

        LogEntry(LOG_MEM,"...Arc mapping allocated");
    }
    else origin = NULL;
}


completeOrientation::~completeOrientation() throw()
{
    if (origin) delete[] origin;

    LogEntry(LOG_MEM,"...Complete orientation disallocated");

    if (CT.traceLevel==2) Display();
}


unsigned long completeOrientation::Size() throw()
{
    return
          sizeof(completeOrientation)
        + managedObject::Allocated()
        + abstractMixedGraph::Allocated()
        + abstractDiGraph::Allocated()
        + diGraph::Allocated()
        + completeOrientation::Allocated();
}


unsigned long completeOrientation::Allocated() throw()
{
    if (origin) return sizeof(TArc)*m;
    else return 0;
}


TArc completeOrientation::OriginalArc(TArc a) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (a>=2*m) NoSuchArc("OriginalArc",a);

    #endif

    if (type==1) return (a>>1);
    if (origin!=NULL) return origin[a>>1];
    return a;
}


void completeOrientation::MapFlowForward(abstractMixedGraph &G) throw(ERRejected)
{
    TArc aG = 0;

    for (TArc a=0;a<G.M();a++)
    {
        TFloat thisFlow = G.Sub(2*a);

        if (thisFlow>0) SetSub(2*aG,thisFlow);

        ++aG;

        if (G.Orientation(2*a)==0)
        {
            #if defined(_FAILSAVE_)

            if (G.LCap(2*a)!=0)
            {
                Error(ERR_REJECTED,"MapFlowForward","Lower capacity bound must be zero");
            }

            #endif

            if (thisFlow<0) SetSub(2*aG,-thisFlow);

            ++aG;
        }
    }
}


void completeOrientation::MapFlowBackward(abstractMixedGraph &G) throw(ERRejected)
{
    TArc aG = 0;

    for (TArc a=0;a<G.M();a++)
    {
        TFloat thisFlow = Sub(2*aG);
        ++aG;

        if (G.Orientation(2*a)==0)
        {
            #if defined(_FAILSAVE_)

            if (G.LCap(2*a)!=0)
            {
                Error(ERR_REJECTED,"MapFlowBackward","Lower capacity bound must be zero");
            }

            #endif

            thisFlow -= Sub(2*aG);
            ++aG;
        }

        G.SetSub(2*a,thisFlow);
    }
}


inducedOrientation::inducedOrientation(abstractMixedGraph &G,TOption options) throw(ERRejected) :
    managedObject(G.Context()),
    diGraph(G.N(),G.Context(),true)
{
    LogEntry(LOG_MAN,"Orienting graph arcs by layers...");

    TIndex* originalArc = (options & OPT_MAPPINGS) ? new TArc[G.M()] : NULL;

    for (TArc a=0;a<2*G.M();a++)
    {
        if (G.NodeColour(G.StartNode(a))<G.NodeColour(G.EndNode(a)))
        {
            TArc a1 = InsertArc(G.StartNode(a),G.EndNode(a),G.UCap(a),G.Length(a&(a^1)));

            if (originalArc) originalArc[a1] = a;
        }
        else if (   (options & OPT_CLONE)
                 && G.NodeColour(G.StartNode(a))==G.NodeColour(G.EndNode(a)) )
        {
            Error(ERR_REJECTED,"inducedOrientation","Invalid node colouring");
        }
    }

    for (TNode v=0;v<n;v++)
    {
        X.SetDemand(v,G.Demand(v));

        for (TDim i=0;i<G.Dim();i++) X.SetC(v,i,G.C(v,i));
    }

    if (options & OPT_CLONE)
    {
        // Preserve planar representation

        for (TNode u=0;u<n;u++)
        {
            TArc a = G.First(u);

            if (a==NoArc) continue;

            do
            {
                TArc a2 = G.Right(a,u);

                TArc revert1 = 0;
                if (StartNode(a)!=G.StartNode(a)) revert1 = 1;

                TArc revert2 = 0;
                if (StartNode(a2)!=G.StartNode(a2)) revert2 = 1;

                X.SetRight(a^revert1,a2^revert2);
                a = a2;
            }
            while (a!=G.First(u));

            TArc revert1 = 0;
            if (StartNode(a)!=G.StartNode(a)) revert1 = 1;

            X.SetFirst(u,a^revert1);
        }

        TArc exteriorArc = G.ExteriorArc();

        if (exteriorArc!=NoArc)
        {
            face = new TNode[2*m];

            for (TArc i=0;i<2*m;i++)
            {
                if (StartNode(i)==G.StartNode(i))
                {
                    face[i] = G.Face(i);
                }
                else
                {
                    face[i] = G.Face(i^1);
                }
            }

            if (StartNode(exteriorArc)!=G.StartNode(exteriorArc))
            {
                exteriorArc ^= 1;
            }

            SetExteriorArc(exteriorArc);
        }
    }

    if (options & OPT_MAPPINGS)
    {
        TIndex* originalArcExport = registers.RawArray<TIndex>(*this,TokRegOriginalArc);
        memcpy(originalArcExport,originalArc,sizeof(TIndex)*m);
        delete[] originalArc;
    }
}


nodeSplitting::nodeSplitting(abstractMixedGraph& _G,TOption options) throw() :
    managedObject(_G.Context()),
    diGraph(2*_G.N(),_G.Context(),true),
    G(_G)
{
    LogEntry(LOG_MAN,"Splitting graph nodes...");

    mapUnderlying = (options & MCC_MAP_UNDERLYING);
    mapCapacities = (options & MCC_MAP_DEMANDS);

    X.SetCapacity(2*G.N(),2*G.M()+G.N());

    for (TArc a=0;a<2*G.M();a++)
    {
        if (!G.Blocking(a) || mapUnderlying)
        {
            InsertArc(2*G.StartNode(a)+1,2*G.EndNode(a),
                mapCapacities ? G.UCap(a) : (G.M()+1)*G.MaxDemand(),0);
        }
    }

    for (TNode v=0;v<G.N();v++)
    {
        InsertArc(2*v,2*v+1,mapCapacities ? G.Demand(v) : 1,0);
    }

    X.SetCapacity(N(),M());

    if (G.Dim()>=2)
    {
        for (TNode v=0;v<G.N();v++)
        {
            X.SetC(2*v,0,(G.C(v,0)));
            X.SetC(2*v,1,(G.C(v,1)));
            X.SetC(2*v+1,0,(G.C(v,0))+5);
            X.SetC(2*v+1,1,(G.C(v,1))+3);
        }
    }

    if (CT.traceLevel==2) Display();
}


regularTree::regularTree(TNode _depth,TNode deg,TNode _n,goblinController &_CT) throw(ERRejected) :
    managedObject(_CT),
    diGraph(TNode(1),_CT)
{
    LogEntry(LOG_MAN,"Generating regular tree...");

    Layout_ConvertModel(LAYOUT_FREESTYLE_POLYGONES);

    TNode n0 = 0;
    TNode n1 = 1;
    TNode depth = 0;

    X.SetC(0,0,0);
    X.SetC(0,1,0);

    while (n<_n && depth<_depth)
    {
        ++depth;
        TNode n2 = n1*deg;
        TFloat r = depth*CT.nodeSep;

        for (TNode k=0;k<n1 && n<_n;++k)
        {
            for (TNode l=0;l<deg && n<_n;l++)
            {
                TNode v = InsertNode();
                InsertArc(n0+k,v);

                X.SetC(v,0,-sin(2*PI*(v-n0-n1+0.5)/n2)*r);
                X.SetC(v,1, cos(2*PI*(v-n0-n1+0.5)/n2)*r);
            }
        }

        n0 += n1;
        n1 = n2;
    }

    IncidenceOrderFromDrawing();
}


directedDual::directedDual(abstractMixedGraph &G,TOption options) throw(ERRejected) :
    managedObject(G.Context()),
    diGraph(G.M()-G.N()+2,G.Context())
{
    #if defined(_FAILSAVE_)

    if (G.M()-G.N()+2>=CT.MaxNode())
        Error(ERR_REJECTED,"directedDual","Number of regions is out of range");

    #endif

    // Save the required st-numbering of the original graph.
    // These are overwrittem by the ExtractEmbedding() procedure
    TNode* stNumber = new TNode[G.N()];

    for (TNode v=0;v<G.N();v++) stNumber[v] = G.NodeColour(v);

    // Place nodes and setup the target node
    TArc extArc = G.ExteriorArc();
    if (G.ExtractEmbedding(PLANEXT_DUAL,
            reinterpret_cast<void*>(static_cast<abstractMixedGraph*>(this)))==NoNode)
    {
        Error(ERR_REJECTED,"directedDual","Input graph is not embedded");
    }

    if (extArc==NoArc) extArc = G.ExteriorArc();

    targetNode = G.Face(extArc);

    // Insert edges
    TArc* map = new TArc[2*G.M()];

    for (TArc a=0;a<G.M();a++)
    {
        TNode u = G.Face(2*a);
        TNode v = G.Face(2*a+1);

        TNode x = G.StartNode(2*a);
        TNode y = G.EndNode(2*a);

        if (   (   (G.Orientation(2*a)==0 || stNumber[x]<stNumber[y])
                && u!=targetNode
               )
            || v==targetNode
           )
        {
            InsertArc(u,v);

            map[2*a]   = 2*a+1;
            map[2*a+1] = 2*a;
        }
        else
        {
            InsertArc(v,u);

            map[2*a]   = 2*a;
            map[2*a+1] = 2*a+1;
        }
    }

    for (TNode v=0;v<G.N();v++) G.SetNodeColour(v,stNumber[v]);

    delete[] stNumber;


    // Set up dual planar representation
    TArc* predArc = new TArc[2*G.M()];

    for (TArc a=0;a<G.M();a++)
    {
        TNode x = G.StartNode(2*a);
        TNode y = G.EndNode(2*a);

        predArc[map[2*a]]   = map[G.Right(2*a,x)];
        predArc[map[2*a+1]] = map[G.Right(2*a+1,y)];
    }

    delete[] map;

    X.ReorderIncidences(predArc);

    delete[] predArc;


    // Determine the source node and the returnArc
    TArc returnArc = NoArc;

    for (TNode v=0;v<n && sourceNode==NoNode;v++)
    {
        TArc a = First(v);

        sourceNode = v;

        do
        {
            if (a&1) sourceNode = NoNode;

            if (EndNode(a)==targetNode) returnArc = a;

            a = Right(a,v);
        }
        while (a!=First(v) && sourceNode==v);
    }

    if (returnArc!=NoArc) MarkExteriorFace(returnArc);


    // if (G.Dim()>=2) AutoArcAlignment();

    if (CT.traceLevel==2) Display();
}


transitiveClosure::transitiveClosure(abstractDiGraph &G,TOption options)
    throw(ERRejected) :
    managedObject(G.Context()),
    diGraph(G.N(),G.Context())
{
    LogEntry(LOG_MAN,"Computing transitive closure...");

    TNode* adjacent = new TNode[n];
    for (TNode w=0;w<n;w++) adjacent[w] = NoNode;

    THandle H = G.Investigate();
    investigator &I = G.Investigator(H);


    // Step 1: Generate a plain copy of G but sort out parallel arcs

    for (TNode u=0;u<n;u++)
    {
        X.SetDemand(u,G.Demand(u));

        for (TDim i=0;i<G.Dim();i++) X.SetC(u,i,G.C(u,i));

        while (I.Active(u))
        {
            TArc a = I.Read(u);
            TNode v = G.EndNode(a);

            if (!(a&1) && adjacent[v]!=u)
            {
                InsertArc(u,v,G.UCap(a),1,G.LCap(a));
                adjacent[v] = u;
            }
        }
    }

    TArc m0 = m;


    // Step 2: Augment to a transitive DAG or report that
    //    the input graph is not a DAG

    for (TNode w=0;w<n;w++) adjacent[w] = NoNode;

    for (TNode u=0;u<n;u++)
    {
        // Determine all descendants of u

        CT.SuppressLogging();
        BFS(SPX_PLAIN,u);
        CT.RestoreLogging();

        I.Reset(u);

        while (I.Active(u))
        {
            TArc a = I.Read(u);
            TNode v = G.EndNode(a);

            if (a&1) continue;

            adjacent[v] = u;
        }

        TFloat* dist = GetDistanceLabels();

        for (TNode v=0;v<n;v++)
        {
            if (v!=u && dist[v]!=InfFloat && adjacent[v]!=u)
            {
                InsertArc(u,v);
            }
        }
    }

    G.Close(H);

    delete[] adjacent;

    X.SetCapacity(N(),M());

    if (options & OPT_SUB)
    {
        TArc* edgeColour = InitEdgeColours();

        for (TArc a=0;a<m;a++)
        {
            edgeColour[a] = (a<m0);
        }
    }

    if (CT.traceLevel==2) Display();
}


intransitiveReduction::intransitiveReduction(abstractDiGraph &G,TOption options)
    throw(ERRejected) :
    managedObject(G.Context()),
    diGraph(G.N(),G.Context())
{
    LogEntry(LOG_MAN,"Computing intransitive reduction...");

    TNode* adjacent = new TNode[n];
    for (TNode w=0;w<n;w++) adjacent[w] = NoNode;

    THandle H = G.Investigate();
    investigator &I = G.Investigator(H);

    // Step 1: Generate a plain copy of G but sort out parallel arcs

    for (TNode u=0;u<n;u++)
    {
        X.SetDemand(u,G.Demand(u));

        for (TDim i=0;i<G.Dim();i++) X.SetC(u,i,G.C(u,i));

        while (I.Active(u))
        {
            TArc a = I.Read(u);
            TNode v = G.EndNode(a);

            if (!(a&1) && adjacent[v]!=u)
            {
                InsertArc(u,v,G.UCap(a),-1,G.LCap(a));
                adjacent[v] = u;
            }
        }
    }

    G.Close(H);

    delete[] adjacent;


    // Step 2: Eliminate all transitive arcs

    H = Investigate();
    investigator &I2 = Investigator(H);

    for (TNode u=0;u<n;u++)
    {
        // Determine maximal paths from u to all of its descendants

        DAGSearch(DAG_SPTREE,u);
        TFloat* dist = GetDistanceLabels();

        while (I2.Active(u))
        {
            TArc a = I2.Read(u);
            TNode v = EndNode(a);

            if (a&1) continue;

            if (dist[v]==dist[u]-1)
            {
                if (options & OPT_SUB) SetEdgeColour(a,1);
            }
            else
            {
                if (options & OPT_SUB)
                {
                    SetEdgeColour(a,0);
                }
                else
                {
                    X.CancelArc(a);
                }
            }
        }
    }

    Close(H);

    X.DeleteArcs();

    X.SetCLength(1);

    X.SetCapacity(N(),M());

    if (CT.traceLevel==2) Display();
}
