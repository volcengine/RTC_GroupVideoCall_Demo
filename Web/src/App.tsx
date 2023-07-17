import { BrowserRouter, Routes, Route } from 'react-router-dom';
import { BASENAME } from './config';
import Login from './pages/Login';
import View from './pages/View';

function App() {
  return (
    <BrowserRouter basename={BASENAME}>
      <Routes>
        <Route path="/">
          <Route index element={<View />} />
          <Route path="login" element={<Login />} />
        </Route>
      </Routes>
    </BrowserRouter>
  );
}

export default App;
