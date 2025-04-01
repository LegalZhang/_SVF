
#ifndef INCLUDE_SVFIR_PAGBUILDERFROMFILE_H_
#define INCLUDE_SVFIR_PAGBUILDERFROMFILE_H_

#include "SVFIR/SVFIR.h"

namespace SVF
{

/*!
 * Build SVFIR from a user specified file (for debugging purpose)
 */
class PAGBuilderFromFile
{

private:
    SVFIR* pag;
    std::string file;
public:
    /// Constructor
    PAGBuilderFromFile(std::string f) :
        pag(SVFIR::getPAG(true)), file(f)
    {
    }
    /// Destructor
    ~PAGBuilderFromFile()
    {
    }

    /// Return SVFIR
    SVFIR* getPAG() const
    {
        return pag;
    }

    /// Return file name
    std::string getFileName() const
    {
        return file;
    }

    /// Start building
    SVFIR* build();

    // Add edges
    void addEdge(NodeID nodeSrc, NodeID nodeDst, APOffset offset,
                 std::string edge);
};

} // End namespace SVF

#endif /* INCLUDE_SVFIR_PAGBUILDERFROMFILE_H_ */
