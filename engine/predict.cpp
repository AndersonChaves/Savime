/*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*    ANDERSON C. SILVA				OCTOBER 2019
*/

#include "include/dml_operators.h"
#include "include/viz.h"
#include "engine/include/predictor.h"

Predict::Predict(OperationPtr operation,
            ConfigurationManagerPtr configurationManager,
            QueryDataManagerPtr queryDataManager,
            MetadataManagerPtr metadataManager, StorageManagerPtr storageManager,
            EnginePtr engine) :
    EngineOperator(operation, configurationManager, queryDataManager, metadataManager, storageManager, engine){

    SET_TARS(_inputTAR, _outputTAR);
    SET_GENERATOR(_generator, _inputTAR->GetName());
    SET_GENERATOR(_outputGenerator, _outputTAR->GetName());
    SET_INT_CONFIG_VAL(_numSubtars, MAX_PARA_SUBTARS);

}

SavimeResult Predict::GenerateSubtar(SubTARIndex subtarIndex){
    /*
     * PARTIAL IMPLEMENTATION
     *  - PREDICT(DOMAIN)
     *    DOMAIN SHOULD CONTAIN THE SPATIAL DOMAIN AS DIMENSIONS AND THE INPUT DATA AS AN ATTRIBUTE
     *    PREDICT SHOULD GET THE SUBTARS FROM DOMAIN AND DO THE FOLLOWING
     *        FOR EVERY X' POINT IN THE SUBTAR
     *            H = DETERMINE_MODEL(X')
     *            VAL = USE_MODEL_TO_PREDICT(H, X')
     *            WRITE VAL TO THE OUTPUT DATASET
     *            BUFFER[X_POSITION] = VAL
     * */

    TARPtr inputTAR = _inputTAR;
    string modelName = _operation->GetParametersByName("model_name")->literal_str;
    string predictedAttribute = _operation->GetParametersByName("attribute")->literal_str;
    auto list = _operation->GetParameters();

    //output TAR is generated by the current operator
    TARPtr outputTAR = _operation->GetResultingTAR();

    //Getting current subTAR
    auto subtar = _generator->GetSubtar(subtarIndex);

    //If subtar is null, return
    if(subtar == nullptr){
        return SAVIME_SUCCESS;
    }

    //Create new subtar
    SubtarPtr newSubtar = make_shared<Subtar>();
    _generator->TestAndDisposeSubtar(subtarIndex);

    //Filling a new subtar with the old datasets
    for (auto entry : subtar->GetDataSets()) {
        if (outputTAR->GetDataElement(entry.first) != nullptr) {
            newSubtar->AddDataSet(entry.first, entry.second);
        }
    }

    //Obtaining Predictions
    auto *p = new Predictor();
    vector<string> predictedValues = p->getPredictions(subtar, _storageManager, modelName, predictedAttribute);
    delete(p);

    newSubtar->AddDataSet("op_result", _storageManager->Create(DOUBLE, predictedValues));

    for (auto entry : subtar->GetDimSpecs()) {
        newSubtar->AddDimensionsSpecification(entry.second);
    }

    //Setting output subTAR equal to the input subTAR
    _outputGenerator->AddSubtar(subtarIndex, newSubtar);

    //Must return success
    return SAVIME_SUCCESS;
}