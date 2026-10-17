# %%
from sklearn.datasets import make_blobs
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
import random
import numpy as np
import pandas as pd
from mpl_toolkits.mplot3d import Axes3D
from scipy.stats import skewnorm
from pathlib import Path
import os

pointsDatasetsPath = Path(__file__).resolve().parent / "points" / "datasets"
pointsQueriesPath = Path(__file__).resolve().parent / "points" / "queries/"
shapesDatasetsPath = Path(__file__).resolve().parent / "shapes" / "datasets"
shapesQueriesPath = Path(__file__).resolve().parent / "shapes" / "queries"

dataSize = 20000000
querySize = 10000
dimensions = 2
random.seed(1)

def makeRangeQueries(numOfQueries, width, height, data):
    queries = []
    for i in range(numOfQueries):
        this_index = random.randint(0, len(data))
        this_point = data[this_index]
        queries.append([this_point[0], this_point[1], this_point[0]+width, this_point[1]+height])
    return queries

def makeSequentialRangeQueries(numOfQueries, outFileName):
    query_extent = 0.01
    queries = []

    for i in range(numOfQueries):
        x = (i + 0.5) * query_extent
        y = (i + 0.5) * query_extent
        if x <= 1 - query_extent and y <= 1 - query_extent:
            queries.append((x - query_extent / 2, y - query_extent / 2, query_extent, query_extent))

    queries = np.array(queries)

    with open(outFileName, 'w') as f:
        for query in queries:
            f.write(f"{query[0]} {query[1]} {query[0]+query[2]} {query[3]+query[1]}\n")

def makeZoomInRangeQueries(numOfQueries, outFileName):
    initial_query_extent = 0.85  
    shrink_factor = 0.98  
    center_point = (0.5, 0.5)
    queries = []
    extent = initial_query_extent

    for i in range(numOfQueries):
        x = center_point[0] - extent / 2
        y = center_point[1] - extent / 2
        queries.append((x, y, extent, extent))
        extent *= shrink_factor

    queries = np.array(queries)

    with open(outFileName, 'w') as f:
        for query in queries:
            f.write(f"{query[0]} {query[1]} {query[0]+query[2]} {query[3]+query[1]}\n")

# def makeExtendedRangeQueries(dataFileName, rangeFileName, outputFileName):
#     data = np.loadtxt(dataFileName)
#     queries = np.loadtxt(rangeFileName)

#     data = np.atleast_2d(data)
#     queries = np.atleast_2d(queries)

#     max_width = np.max(data[:, 2] - data[:, 0])
#     max_height = np.max(data[:, 3] - data[:, 1])

#     queries[:, 0] = np.maximum(queries[:, 0] - max_width, 0)
#     queries[:, 1] = np.maximum(queries[:, 1] - max_height, 0)

#     np.savetxt(outputFileName, queries, fmt="%g")

def makeExtendedRangeQueries(dataFileName, rangeFileName, outputFileName):
    data = np.atleast_2d(np.loadtxt(dataFileName, dtype=np.float64))
    queries = np.atleast_2d(np.loadtxt(rangeFileName, dtype=np.float64))

    max_width = np.max(data[:, 2] - data[:, 0])
    max_height = np.max(data[:, 3] - data[:, 1])

    queries[:, 0] = np.maximum(queries[:, 0] - max_width, 0.0)
    queries[:, 1] = np.maximum(queries[:, 1] - max_height, 0.0)

    np.savetxt(outputFileName, queries, fmt="%.17g")

def makeKNNQueries(numOfQueries, dataFileName, queryFileName):
    data = np.loadtxt(dataFileName)
    indices = np.random.default_rng(1).integers(0, len(data), size=numOfQueries)
    queries = data[indices]
    np.savetxt(queryFileName, queries, fmt="%g")

def makeSequentialKNNQueries(D, outFileName, query_size=0.01):
    numQueries = int(1.0 / query_size)

    coordinates = (
        (np.arange(numQueries) + 0.5) * query_size
    )

    queries = np.repeat(coordinates[:, None], D, axis=1)

    np.savetxt(outFileName, queries, fmt="%g")

def makeConcentrateKNNQueries(
    D,
    outFileName,
    numQueries=1000,
    initial_query_size=0.85,
    final_query_size=0.05,
    alpha=1.0,
    seed=1
):
    rng = np.random.default_rng(seed)

    t = np.arange(numQueries) / (numQueries - 1)

    sizes = (
        final_query_size
        + (initial_query_size - final_query_size)
        * (1.0 - t) ** alpha
    )

    low = 0.5 - sizes / 2.0
    high = 0.5 + sizes / 2.0

    queries = rng.uniform(
        low[:, None],
        high[:, None],
        size=(numQueries, D)
    )

    np.savetxt(outFileName, queries, fmt="%g")

def makeMixedQueries(rangeFileName, nnFileName, mixedFileName):
    rangeQueries = np.atleast_2d(np.loadtxt(rangeFileName))
    nnQueries = np.atleast_2d(np.loadtxt(nnFileName))

    numQueries = min(len(rangeQueries), len(nnQueries)) // 2

    rangeQueries = rangeQueries[:numQueries]
    nnQueries = nnQueries[:numQueries]

    with open(mixedFileName, "w") as f:
        for r, nn in zip(rangeQueries, nnQueries):
            f.write(f'RANGE {" ".join(map(str, r))}\n')
            f.write(f'KNN {nn[0]} {nn[1]} {nn[0]} {nn[1]}\n')

def writeToFile(data, filename):
    out = open(filename,'w')
    for i in range(len(data)):
        for j in range(len(data[i])):
        # items = data[i].split()
            out.write(f'{data[i][j]} ')
        out.write('\n')
    out.close()

def writeDataForAir(inFile, filename):
    out = open(filename,'w')
    with open(inFile) as f:
        for line in f:
            items = line.split()
            out.write(f'{items[0]} {items[1]} {items[0]} {items[1]}\n')
    out.close()

def normalizePointData(queryFile, dataFile, dataNormFile, queryNormFile):
    df = pd.read_csv(queryFile,sep=' ',header=None)
    df1 = pd.read_csv(dataFile,sep=' ',header=None)

    data = df1.values
    x, y = data[:,0], data[:,1]

    queries = df.values
    xStart, yStart, xEnd, yEnd = queries[:,0], queries[:,1], queries[:,2], queries[:,3]

    dMinx = np.min(x)
    dMiny = np.min(y)
    dMaxx = np.max(x)
    dMaxy = np.max(y)

    qMinX = np.min((np.min(xStart),np.min(xEnd)))
    qMinY = np.min((np.min(yStart),np.min(yEnd)))
    qMaxX = np.max((np.max(xStart),np.max(xEnd)))
    qMaxY = np.max((np.max(yStart),np.max(yEnd)))

    minx = np.min((dMinx,qMinX))
    miny = np.min((dMiny,qMinY))
    maxx = np.max((dMaxx,qMaxX))
    maxy = np.max((dMaxy,qMaxY))

    xExtent = maxx - minx
    yExtent = maxy - miny

    xNorm = (x - minx)/xExtent
    yNorm = (y - miny)/yExtent

    qxStartNorm = (xStart - minx)/xExtent
    qyStartNorm = (yStart - miny)/yExtent
    qxEndNorm = (xEnd - minx)/xExtent
    qyEndNorm = (yEnd - miny)/yExtent

    out = open(dataNormFile,'w')
    for i in range(len(xNorm)):
        out.write(f'{xNorm[i]} {yNorm[i]}\n')
    out.close()

    out = open(queryNormFile,'w')
    for i in range(len(qxEndNorm)):
        out.write(f'{qxStartNorm[i]} {qyStartNorm[i]} {qxEndNorm[i]} {qyEndNorm[i]}\n')
    out.close()

def normalizeShapeData(queryFile, dataFile, dataNormFile, queryNormFile):
    df = pd.read_csv(queryFile,sep=' ',header=None)
    df1 = pd.read_csv(dataFile,sep=' ',header=None)

    data = df1.values
    xStart, yStart, xEnd, yEnd = data[:,0], data[:,1], data[:,2], data[:,3]

    queries = df.values
    qxStart, qyStart, qxEnd, qyEnd = queries[:,0], queries[:,1], queries[:,2], queries[:,3]

    dMinX = np.min((np.min(xStart),np.min(xEnd)))
    dMinY = np.min((np.min(yStart),np.min(yEnd)))
    dMaxX = np.max((np.max(xStart),np.max(xEnd)))
    dMaxY = np.max((np.max(yStart),np.max(yEnd)))

    qMinX = np.min((np.min(qxStart),np.min(qxEnd)))
    qMinY = np.min((np.min(qyStart),np.min(qyEnd)))
    qMaxX = np.max((np.max(qxStart),np.max(qxEnd)))
    qMaxY = np.max((np.max(qyStart),np.max(qyEnd)))

    minx = np.min((dMinX,qMinX))
    miny = np.min((dMinY,qMinY))
    maxx = np.max((dMaxX,qMaxX))
    maxy = np.max((dMaxY,qMaxY))

    xExtent = maxx - minx
    yExtent = maxy - miny

    dxStartNorm = (xStart - minx)/xExtent
    dyStartNorm = (yStart - miny)/yExtent
    dxEndNorm = (xEnd - minx)/xExtent
    dyEndNorm = (yEnd - miny)/yExtent

    qxStartNorm = (qxStart - minx)/xExtent
    qyStartNorm = (qyStart - miny)/yExtent
    qxEndNorm = (qxEnd - minx)/xExtent
    qyEndNorm = (qyEnd - miny)/yExtent

    out = open(dataNormFile,'w')
    for i in range(len(dxStartNorm)):
       out.write(f'{dxStartNorm[i]} {dyStartNorm[i]} {dxEndNorm[i]} {dyEndNorm[i]}\n')
    out.close()

    out = open(queryNormFile,'w')
    for i in range(len(qxEndNorm)):
        out.write(f'{qxStartNorm[i]} {qyStartNorm[i]} {qxEndNorm[i]} {qyEndNorm[i]}\n')
    out.close()

# %%
##### Uniform point data and query generation #####
points = np.random.uniform(0, 1, (dataSize, dimensions))
uniformData = list(zip(points[:, 0],points[:, 1]))
uniformRangeQueries = makeRangeQueries(querySize, 0.01, 0.01, uniformData)
writeToFile(uniformData, pointsDatasetsPath / "uniform_20M_2D.txt")
writeToFile(uniformRangeQueries, pointsQueriesPath / "uniform_10K_2D_range_queries.txt")
normalizePointData(pointsQueriesPath/"uniform_10K_2D_range_queries.txt",pointsDatasetsPath/"uniform_20M_2D.txt", pointsDatasetsPath/"uniform_20M_2D_norm.txt", pointsQueriesPath/"uniform_10K_2D_range_queries_norm.txt")
writeDataForAir(pointsDatasetsPath/'uniform_20M_2D_norm.txt', pointsDatasetsPath/'uniform_20M_2D_norm_air.txt')
makeKNNQueries(querySize, pointsDatasetsPath/"uniform_20M_2D_norm.txt", pointsQueriesPath/"uniform_10K_2D_knn_queries_norm.txt")
makeMixedQueries(pointsQueriesPath/"uniform_10K_2D_range_queries_norm.txt", pointsQueriesPath/"uniform_10K_2D_knn_queries_norm.txt", pointsQueriesPath/"uniform_10K_2D_mixed_queries_norm.txt")

os.remove(pointsDatasetsPath/'uniform_20M_2D.txt')
os.remove(pointsQueriesPath/'uniform_10K_2D_range_queries.txt')

# %%
##### Skewed point data and query generation #####
alpha = 20
skewedData = skewnorm.rvs(alpha, size=(dataSize, dimensions))
skewedRangeQueries = makeRangeQueries(querySize, 0.02, 0.02, skewedData)
writeToFile(skewedData, pointsDatasetsPath / "skewed_20M_2D.txt")
writeToFile(skewedRangeQueries, pointsQueriesPath / "skewed_10K_2D_range_queries.txt")
normalizePointData(pointsQueriesPath/"skewed_10K_2D_range_queries.txt",pointsDatasetsPath/"skewed_20M_2D.txt", pointsDatasetsPath/"skewed_20M_2D_norm.txt", pointsQueriesPath/"skewed_10K_2D_range_queries_norm.txt")
writeDataForAir(pointsDatasetsPath/'skewed_20M_2D_norm.txt', pointsDatasetsPath/'skewed_20M_2D_norm_air.txt')
makeKNNQueries(querySize, pointsDatasetsPath/"skewed_20M_2D_norm.txt", pointsQueriesPath/"skewed_10K_2D_knn_queries_norm.txt")
makeMixedQueries(pointsQueriesPath/"skewed_10K_2D_range_queries_norm.txt", pointsQueriesPath/"skewed_10K_2D_knn_queries_norm.txt", pointsQueriesPath/"skewed_10K_2D_mixed_queries_norm.txt")
os.remove(pointsDatasetsPath/'skewed_20M_2D.txt')
os.remove(pointsQueriesPath/'skewed_10K_2D_range_queries.txt')
# %%
##### Clustered point data and query generation #####
np.random.seed(42)

n_clusters = 5

centers = np.array([[0.2, 0.2],
                    [0.2, 0.8],
                    [0.8, 0.2],
                    [0.8, 0.8],
                    [0.5, 0.5]])  

X, y = make_blobs(n_samples=dataSize, centers=centers, cluster_std=0.04, random_state=42)
clusteredData = list(zip(X[:,0].tolist(),X[:,1].tolist()))
clusteredRangeQueries = makeRangeQueries(querySize, 0.0034, 0.0034, clusteredData)
writeToFile(clusteredData, pointsDatasetsPath / "clustered_20M_2D.txt")
writeToFile(clusteredRangeQueries, pointsQueriesPath / "clustered_10K_2D_range_queries.txt")
normalizePointData(pointsQueriesPath/"clustered_10K_2D_range_queries.txt",pointsDatasetsPath/"clustered_20M_2D.txt", pointsDatasetsPath/"clustered_20M_2D_norm.txt", pointsQueriesPath/"clustered_10K_2D_range_queries_norm.txt")
writeDataForAir(pointsDatasetsPath/'clustered_20M_2D_norm.txt', pointsDatasetsPath/'clustered_20M_2D_norm_air.txt')
makeKNNQueries(querySize, pointsDatasetsPath/"clustered_20M_2D_norm.txt", pointsQueriesPath/"clustered_10K_2D_knn_queries_norm.txt")
makeMixedQueries(pointsQueriesPath/"clustered_10K_2D_range_queries_norm.txt", pointsQueriesPath/"clustered_10K_2D_knn_queries_norm.txt", pointsQueriesPath/"clustered_10K_2D_mixed_queries_norm.txt")
os.remove(pointsDatasetsPath/'clustered_20M_2D.txt')
os.remove(pointsQueriesPath/'clustered_10K_2D_range_queries.txt')

# %%
##### Uniform shape data and query generation #####
center_points = np.random.uniform(0, 1, (dataSize, dimensions))
shapes = []
extension_range = np.random.uniform(0, 0.002, (dataSize, dimensions))  # Extensions in [0, 0.002]

uniformShapeData = []
for center, extension in zip(center_points, extension_range):
    shape = np.array([
        center - extension,
        center + extension 
    ])
    uniformShapeData.append([shape[0][0], shape[0][1], shape[1][0], shape[1][1]])

uniformShapeQueries = makeRangeQueries(querySize, 0.01, 0.01, uniformShapeData)
writeToFile(uniformShapeData, shapesDatasetsPath / "uniform_20M_2D.txt")
writeToFile(uniformShapeQueries, shapesQueriesPath / "uniform_10K_2D_range_queries.txt")
normalizeShapeData(shapesQueriesPath/"uniform_10K_2D_range_queries.txt",shapesDatasetsPath/"uniform_20M_2D.txt", shapesDatasetsPath/"uniform_20M_2D_norm.txt", shapesQueriesPath/"uniform_10K_2D_range_queries_norm.txt")

os.remove(shapesDatasetsPath/'uniform_20M_2D.txt')
os.remove(shapesQueriesPath/'uniform_10K_2D_range_queries.txt')
# %%
##### Skewed shape data and query generation #####
alpha = 20
center_points = skewnorm.rvs(alpha, size=(dataSize, dimensions))

shapes = []
extension_range = np.random.uniform(0, 0.002, (dataSize, dimensions))  # Extensions in [0, 0.002]

skewedShapeData = []
for center, extension in zip(center_points, extension_range):
    shape = np.array([
        center - extension,   # Corner 2 (min)
        center + extension  # Corner 1 (max)
    ])
    skewedShapeData.append([shape[0][0], shape[0][1], shape[1][0], shape[1][1]])

skewedShapeQueries = makeRangeQueries(querySize, 0.02, 0.02, skewedShapeData)

writeToFile(skewedShapeData, shapesDatasetsPath / "skewed_20M_2D.txt")
writeToFile(skewedShapeQueries, shapesQueriesPath / "skewed_10K_2D_range_queries.txt")
normalizeShapeData(shapesQueriesPath/"skewed_10K_2D_range_queries.txt",shapesDatasetsPath/"skewed_20M_2D.txt", shapesDatasetsPath/"skewed_20M_2D_norm.txt", shapesQueriesPath/"skewed_10K_2D_range_queries_norm.txt")

os.remove(shapesDatasetsPath/'skewed_20M_2D.txt')
os.remove(shapesQueriesPath/'skewed_10K_2D_range_queries.txt')


# %%
##### Clustered shape data and query generation #####

np.random.seed(42)
n_clusters = 5

centers = np.array([[0.2, 0.2],
                    [0.2, 0.8],
                    [0.8, 0.2],
                    [0.8, 0.8],
                    [0.5, 0.5]])  

X, y = make_blobs(n_samples=dataSize, centers=centers, cluster_std=0.04, random_state=42)

width = 0.002
height = 0.002
data = list(zip(X[:,0].tolist(),X[:,1].tolist()))
clusteredShapeData = []
for i in range(len(data)):
    rndWidth = random.uniform(0,width)
    rndHeight = random.uniform(0,height)
    clusteredShapeData.append([data[i][0]-rndWidth, data[i][1]-rndHeight, data[i][0]+rndWidth, data[i][1]+rndHeight])

clusteredShapeQueries = makeRangeQueries(querySize, 0.0001, 0.00001, clusteredShapeData)

writeToFile(clusteredShapeData, shapesDatasetsPath / "clustered_20M_2D.txt")
writeToFile(clusteredShapeQueries, shapesQueriesPath / "clustered_10K_2D_range_queries.txt")
normalizeShapeData(shapesQueriesPath/"clustered_10K_2D_range_queries.txt",shapesDatasetsPath/"clustered_20M_2D.txt", shapesDatasetsPath/"clustered_20M_2D_norm.txt", shapesQueriesPath/"clustered_10K_2D_range_queries_norm.txt")

os.remove(shapesDatasetsPath/'clustered_20M_2D.txt')
os.remove(shapesQueriesPath/'clustered_10K_2D_range_queries.txt')



# %%
##### Sequential and zoomIn range query workloads generation #####
import shutil
makeSequentialRangeQueries(querySize, pointsQueriesPath/"sequential_100_2D_range_queries_norm.txt")
makeZoomInRangeQueries(1000, pointsQueriesPath/"zoomin_1K_2D_range_queries_norm.txt")

shutil.copy(pointsQueriesPath/"sequential_100_2D_range_queries_norm.txt", shapesQueriesPath/"sequential_100_2D_range_queries_norm.txt")
shutil.copy(pointsQueriesPath/"zoomin_1K_2D_range_queries_norm.txt", shapesQueriesPath/"zoomin_1K_2D_range_queries_norm.txt")

# %%
##### Sequential and Concentrate kNN query workloads generation #####
import shutil
makeSequentialKNNQueries(dimensions, pointsQueriesPath/"sequential_100_2D_kNN_queries_norm.txt")
makeConcentrateKNNQueries(dimensions, pointsQueriesPath/"concentrate_1K_2D_kNN_queries_norm.txt")

# %%
makeExtendedRangeQueries(shapesDatasetsPath/"uniform_20M_2D_norm.txt", shapesQueriesPath/"uniform_10K_2D_range_queries_norm.txt", shapesQueriesPath/"uniform_10K_2D_range_queries_norm_qwe.txt")
makeExtendedRangeQueries(shapesDatasetsPath/"clustered_20M_2D_norm.txt", shapesQueriesPath/"clustered_10K_2D_range_queries_norm.txt", shapesQueriesPath/"clustered_10K_2D_range_queries_norm_qwe.txt")
makeExtendedRangeQueries(shapesDatasetsPath/"skewed_20M_2D_norm.txt", shapesQueriesPath/"skewed_10K_2D_range_queries_norm.txt", shapesQueriesPath/"skewed_10K_2D_range_queries_norm_qwe.txt")
# %%
makeExtendedRangeQueries(shapesDatasetsPath/"uniform_20M_2D_norm.txt", shapesQueriesPath/"sequential_100_2D_range_queries_norm.txt", shapesQueriesPath/"sequential_100_2D_range_queries_norm_qwe.txt")
makeExtendedRangeQueries(shapesDatasetsPath/"uniform_20M_2D_norm.txt", shapesQueriesPath/"zoomin_1K_2D_range_queries_norm.txt", shapesQueriesPath/"zoomin_1K_2D_range_queries_norm_qwe.txt")

# %%
