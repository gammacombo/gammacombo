Gammacombo for Datasets - State of development

There is a branch in the gammacombo git repository, called `datasets_dev`. The master version of gammacombo can only combine a list of single measurements. Having more than a dozen of measurements can become very tedious.
The `datasets_dev` branch aims to add functionality to gammacombo so it additionally can take on (or more) a whole dataset as input. For example, one might want to use the mass of a sample of events as input. Each dataset may have a huge number of events, but the events in each dataset must be independent and identically distributed. This is an important extension in functionality, for example if on wants to use gammacombo non-trivial limit setting.



Work on this started when Max was doing a combined measurement of the Bs2MuMu branching ratio between LHCb and CMS. He started with an older version of the gammacombo master and adapted it to the needs of the Bs2MuMu analysis. However, since he needed to complete the analysis, he had no time to focus on making his adaptation of gammacbombo usable for others. There was no clear border between the analysis-specific code that should go into the "combiner" module and code that should be shared between analyses and be part of gammacombo. The interface which was exposed was very inconsistent with the interface used in the master branch of gammacombo. 

In spring 2016, Max and I went back to revisit his code. We created the `datasets_dev` branch in the git repository to hold his version of gammacombo until it is merged into the master. We worked to define a clear distinction beween core gammacombo functionality and code that belongs into the analysis-specific combiners. We defined an interface which supports additional commands for the datasets - functionality but is otherwise compatible with the interface in the `master`. And we added an additional tutorial which shows how to use the gammacombo with a dataset. I also stated to refactor: Clean up the code, improve readability and maintainability. Some progress has been made, but we are still not quite there yet. Max left science shortly after, my priorities shifted to work on the analysis for my PhD.

We wanted to verify that our changes so far had not broken anything. To verify this, we were aiming to run Gammacombo again for the Bs2MuMu Analysis. This cross-check was started, but not completed.. It is very conputing-intensive and requires access to the files of the Bs2MuMu analysis. But even with as far as we got with the crosscheck, it seems that it was NOT successful. There is no good match with the plot that Max showed in his thesis and there is an overly big disagreement between the Prob-scan dna dthe Plugin-scan results. This can have many reasons. It might just be that the very complex fit is somehow misbehaving with the newer version of root we are using, or that a subtle change we made as broken it. Not necessarily does it mean that the datasets branch of gammacombo is broken in the current state, especially that for the dataset-example in the tutorial, no such discrepancy can be seen.

Now I am also dropping my PhD and leaving science. 

My email address is: schubert.konstantin@gmail.com
Max' email address is: maxschlupp@gmail.com
