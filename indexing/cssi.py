#! /usr/bin/python3.9
import gensim.downloader
import os, sys
import numpy as np
import pandas as pd
from sklearn.cluster import KMeans
from sklearn.decomposition import PCA
from tqdm.auto import tqdm
from scipy.spatial.distance import cdist
import time
import pickle
import shutil

def get_maxdist(a):
    '''
    This is used in order to obtain the max possible distance in sample (which is used to normalize all distances)
    '''
    return cdist(a.min(axis=0).reshape(1,-1), a.max(axis=0).reshape(1,-1))

def getr(data, ccs, labels, L, metric, maxt):
    '''
    This returns all the radii for a given set of clusters
    '''
    return [np.max(cdist(ccs[i].reshape(1,-1), data[labels==i], metric=metric)/maxt) for i in range(L)]

def get_emb(txt, model, stopwords=False, swlist=[]):
    '''
    This function is supplied to a pandas apply and creates the embeddings of each sentence in 'text' column
    '''
    embs = []
    sws = []
    try:
        for word in txt.split():
            if word.lower() in swlist:
                sws.append(model[word.lower()])
                continue 
            try:
                embs.append(model[word.lower()])
            except:
                pass
        if len(embs)>2: # if there are enough words in the dictionary
            if stopwords: # if this is true include stopwords
                return np.mean(np.array(embs+sws), axis=0) 
            else:
                return np.mean(np.array(embs), axis=0)
        else:
            return None
    except:
        return None

dataset = os.getenv('DATASET', 'twitter') 
N = int(os.getenv('N', -1)) # sample to be read - leave -1 if you want to read the whole file
model_name = os.getenv('MODEL', 'glove-twitter-100') # gensim model name
d = model_name.split("-")[-1] # dimensionality - inferred from gensim model 
data = os.getenv('INPUT', f'{dataset}.csv')
stopwords = open('stopwords.txt').read().splitlines() # file that includes the list of stopwords

print('Loading model')
model = gensim.downloader.load(model_name)
model.init_sims(replace=True)
tqdm.pandas()
print('Reading csv')
df = pd.read_csv(data)[:N].dropna()

df['embeddings'] = df.text.progress_apply(lambda txt: get_emb(txt, model, False, stopwords))
df = df.loc[[emb is not None for emb in  df.embeddings]] # only keep the records with embeddings - not None
af = np.vstack(df.embeddings)
print('Embedding shape:', af.shape)
bf = df[['normLongitude', 'normLatitude']].astype(float).values

params = [(10000,0.3,1.0,2)] # you can add extra tuples in this list in order to create many indexes in one run - useful since creating data over and over is slow
for (size,F,sample,m) in params:
    # m = 2
    ao = af[:size]
    b = bf[:size]
    
    st = time.time()
    pca = PCA(m)
    a = pca.fit_transform(ao)
    pca_time = time.time()-st
    print('PCA done in:', pca_time)
    
    st = time.time()
    tmax = get_maxdist(a)
    tomax = get_maxdist(ao)
    smax = get_maxdist(b)
    maxes_time = time.time()-st
    print('Calc max done in:', maxes_time)

    print('Saving data set')
    dataset_savedir = f'{dataset}_dataset/{size}_{d}_{m}'
    try:
        os.mkdir(f'{dataset}_dataset')
    except:
        pass

    try:
        os.mkdir(dataset_savedir)
    except:
        pass

    np.savetxt(f'{dataset_savedir}/a_hd.txt', ao.astype(np.float64), delimiter=' ')
    np.savetxt(f'{dataset_savedir}/a_ld.txt', a.astype(np.float64), delimiter=' ')
    np.savetxt(f'{dataset_savedir}/b.txt', b.astype(np.float64), delimiter=' ')
    np.savetxt(f'{dataset_savedir}/maxes.txt', np.vstack((tmax,tomax,smax)).astype(np.float64), delimiter=' ')

    L = int(np.sqrt(0.01*size)*F) # number of clusters for each field (spatial and textual)
    print('L is ', L) 

    st = time.time()
    if sample!=1.0: # if sample is less than 1.0, use the sample specified for KMeans and add all the other samples later (using "predict")
        kmb = KMeans(L)
        kmb.fit(b[:int(size*sample)])

        kma = KMeans(L)
        kma.fit(a[:int(size*sample)])

        kmblb = np.hstack((kmb.labels_,kmb.predict(b[int(size*sample):])))
        kmalb = np.hstack((kma.labels_,kma.predict(a[int(size*sample):])))
    else:
        kmb = KMeans(L)
        kmb.fit(b)
        kma = KMeans(L)
        kma.fit(a)

        kmblb = kmb.labels_
        kmalb = kma.labels_

    km_time = time.time()-st
    print('KMeans done in:', km_time)
    

    st = time.time()

    ccahd = [np.mean(ao[kmalb==i],axis=0) for i in range(L)] # high-dim cluster centers 

    ra = getr(a, kma.cluster_centers_, kmalb, L, metric='euclidean', maxt=tmax)
    rao = getr(ao, ccahd, kmalb, L, metric='euclidean', maxt=tomax)
    rb = getr(b, kmb.cluster_centers_, kmblb, L, metric='euclidean', maxt=smax)


    combinations = []
    ras = []
    raos = []
    rbs = []
    centers_a = []
    centers_ao = []
    centers_b = []

    for i in range(L):
        for j in range(L):
            # for each hybrid cluster, find the indexes of elements that it includes and save these as well as the radii and cluster centers (low and high dim)
            positions = np.where([(kmalb==i)&(kmblb==j)])[1]
            if len(positions)==0:
                continue
            combinations.append(positions)
            ras.append(ra[i])
            raos.append(rao[i])
            rbs.append(rb[j])
            centers_a.append(kma.cluster_centers_[i].reshape(1,-1))
            centers_ao.append(ccahd[i].reshape(1,-1))
            centers_b.append(kmb.cluster_centers_[j].reshape(1,-1))


    centers_a = np.squeeze(np.array(centers_a)) if a.shape[1]!=1 else np.squeeze(np.array(centers_a)).reshape(-1,1)
    centers_ao = np.squeeze(np.array(centers_ao)) if a.shape[1]!=1 else np.squeeze(np.array(centers_ao)).reshape(-1,1)
    centers_b = np.squeeze(np.array(centers_b))
    ras = np.array(ras)
    rbs = np.array(rbs)
    raos = np.array(raos)

    # the distance matrix that contains the distance from each object to it's hybrid cluster center
    dm = []
    for i in range(len(ao)):
        dm.append((cdist(ao[i].reshape(1,-1), ccahd[kmalb[i]].reshape(1,-1), metric='euclidean')/tomax + cdist(b[i].reshape(1,-1), kmb.cluster_centers_[kmblb[i]].reshape(1,-1), metric='euclidean')/smax)[0])
    dm = np.array(dm)

    rest_time = time.time()-st
    print('Hybrid CLusters done in:', rest_time)
    
    print('Saving Indexes')
    index_savedir = f'{dataset}_indexes/{size}_{d}_{m}_{F}'

    try:
        os.mkdir(f'{dataset}_indexes')
    except:
        pass

    try:
        os.mkdir(index_savedir)
    except:
        pass

    combs = ''
    for i, comb in enumerate(combinations):
        combs+=str(i)+'\n'
        combs+=str(len(comb))+'\n'
        combs+='\n'.join([str(val) for val in comb])+'\n'
    open(os.path.join(index_savedir,f'comb.txt'), 'wt', encoding='utf-8').write(combs[:-1])

    np.savetxt(os.path.join(index_savedir,'ras.txt'), ras.astype(np.float64), delimiter=' ')
    np.savetxt(os.path.join(index_savedir,'raos.txt'), raos.astype(np.float64), delimiter=' ')
    np.savetxt(os.path.join(index_savedir,'rbs.txt'), rbs.astype(np.float64), delimiter=' ')
    np.savetxt(os.path.join(index_savedir,'ca.txt'), centers_a.astype(np.float64), delimiter=' ')
    np.savetxt(os.path.join(index_savedir,'cao.txt'), centers_ao.astype(np.float64), delimiter=' ')
    np.savetxt(os.path.join(index_savedir,'cb.txt'), centers_b.astype(np.float64), delimiter=' ')
    np.savetxt(os.path.join(index_savedir,'dm.txt'), dm.astype(np.float64), delimiter=' ')
    np.savetxt(os.path.join(index_savedir,'misc.txt'), np.vstack((tmax,tomax,smax,len(combinations))).astype(np.float64), delimiter=' ')