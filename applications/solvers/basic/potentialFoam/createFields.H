// This hack method was originally developed by Symscape, circa 2009 -------
// This has been implemented in potentialFoam in 2016 by blueCAPE Lda for
// blueCFD-Core 2016-1
#if defined( WIN32 ) || defined( WIN64 )
// Rename Phi to Phi_ to prevent name clashes on Windows with phi
IOobject::replaceFileName("Phi", "Phi_");
#endif
// End of hack --------------------------------------------

Info<< "Reading velocity field U\n" << endl;
volVectorField U
(
    IOobject
    (
        "U",
        runTime.timeName(),
        mesh,
        IOobject::MUST_READ,
        IOobject::AUTO_WRITE
    ),
    mesh
);

U = dimensionedVector("0", U.dimensions(), Zero);

surfaceScalarField phi
(
    IOobject
    (
        "phi",
        runTime.timeName(),
        mesh,
        IOobject::NO_READ,
        IOobject::AUTO_WRITE
    ),
    fvc::flux(U)
);

if (args.optionFound("initialiseUBCs"))
{
    U.correctBoundaryConditions();
    phi = fvc::flux(U);
}


// Default name for the pressure field
word pName("p");

// Update name of the pressure field from the command-line option
args.optionReadIfPresent("pName", pName);

// Infer the pressure BCs from the velocity
wordList pBCTypes
(
    U.boundaryField().size(),
    fixedValueFvPatchScalarField::typeName
);

forAll(U.boundaryField(), patchi)
{
    if (U.boundaryField()[patchi].fixesValue())
    {
        pBCTypes[patchi] = zeroGradientFvPatchScalarField::typeName;
    }
}

Info<< "Constructing pressure field " << pName << nl << endl;
volScalarField p
(
    IOobject
    (
        pName,
        runTime.timeName(),
        mesh,
        IOobject::READ_IF_PRESENT,
        IOobject::NO_WRITE
    ),
    mesh,
    dimensionedScalar(pName, sqr(dimVelocity), 0),
    pBCTypes
);

// Infer the velocity potential BCs from the pressure
wordList PhiBCTypes
(
    p.boundaryField().size(),
    zeroGradientFvPatchScalarField::typeName
);

forAll(p.boundaryField(), patchi)
{
    if (p.boundaryField()[patchi].fixesValue())
    {
        PhiBCTypes[patchi] = fixedValueFvPatchScalarField::typeName;
    }
}

Info<< "Constructing velocity potential field Phi\n" << endl;
volScalarField Phi
(
    IOobject
    (
        "Phi",
        runTime.timeName(),
        mesh,
        IOobject::READ_IF_PRESENT,
        IOobject::NO_WRITE
    ),
    mesh,
    dimensionedScalar("Phi", dimLength*dimVelocity, 0),
    PhiBCTypes
);

label PhiRefCell = 0;
scalar PhiRefValue = 0;
setRefCell
(
    Phi,
    potentialFlow.dict(),
    PhiRefCell,
    PhiRefValue
);
mesh.setFluxRequired(Phi.name());

#include "createMRF.H"
