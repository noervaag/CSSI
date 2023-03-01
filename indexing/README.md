# Requirements

We suggest using python3.9. You can run the followinfg pip command to install everything that is needed.

```bash
python3.9 -m pip install numpy pandas scikit-learn tqdm scipy gensim
```

# Indexing 

Use cssi.py to create an index.  You can can use the following env variables:
- DATASET -> name of the csv containing normLatitude, normLongitude and Text columns - should be "twitter", "yelp" etc.
- N -> number of rows to be read from the csv mentioned above - useful if you want to subsample a larger file
- MODEL -> model name (see pretrained gensim models for more details)

# Included example

Using the including 100K sample of tweets (twitter.csv), you can easily create the data and index that are needed in order to reproduce our experiments. Just run 

```bash
python3.9 cssi.py
```
